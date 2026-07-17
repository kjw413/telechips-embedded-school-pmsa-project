import warnings
warnings.filterwarnings("ignore", category=FutureWarning)

import logging
logging.basicConfig(level=logging.ERROR)

import sys
import cv2
import torch
import numpy as np
import time
from queue import Queue
import threading

import os
import fcntl
import struct

# IPC 디바이스 파일
IPC_DEVICE = "/dev/tcc_ipc_micom"

# TCC_IPC_MAGIC과 IOCTL_IPC_DETECT_DATA 정의
TCC_IPC_MAGIC = 'I'  
IOCTL_IPC_DETECT_DATA = (ord(TCC_IPC_MAGIC) << 8) | 6

# YOLOv5 모델 로드
path = '/home/root/best2.pt'  # 모델 경로
model = torch.hub.load('ultralytics/yolov5', 'custom', path=path)
classes = ['red']  # 클래스 이름 정의

# 비디오 캡처 설정
cap = cv2.VideoCapture(1)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)  # 해상도 설정
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
cap.set(cv2.CAP_PROP_FPS, 15)  # FPS 제한

# 프레임 큐 생성
frame_queue = Queue(maxsize=1)
result_queue = Queue(maxsize=1)

# IPC로 데이터 전송
def send_to_ipc(fd, value):
    try:
        packed_value = struct.pack('i', int(value))
        fcntl.ioctl(fd, IOCTL_IPC_DETECT_DATA, packed_value)
        print(f"Sent detect data to IPC: {value}")
    except Exception as e:
        print(f"Error sending to IPC: {e}")

# 프레임 캡처 함수
def capture_frames():
    """프레임을 캡처하여 큐에 추가"""
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        if not frame_queue.full():  # 큐가 가득 찼는지 확인하여 불필요한 프레임 무시
            frame_queue.put(frame)

# 프레임 처리 함수
def process_frames():
    """프레임 처리 및 YOLOv5 추론"""
    while True:
        if not frame_queue.empty():
            frame = frame_queue.get()
            frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)  # BGR -> RGB 변환
            results = model(frame_rgb)  # YOLOv5 추론
            if not result_queue.full():  # 결과 큐가 가득 찼는지 확인하여 불필요한 결과 무시
                result_queue.put((frame, results))  # 결과를 큐에 추가

# IPC 디바이스 열기
fd = os.open(IPC_DEVICE, os.O_WRONLY | os.O_NONBLOCK)
print(f"Device {IPC_DEVICE} is open. File descriptor: {fd}")

# 스레드 생성
capture_thread = threading.Thread(target=capture_frames)
process_thread = threading.Thread(target=process_frames)
capture_thread.start()
process_thread.start()

# 이전 상태 저장 변수
recent_detections = []  # 최근 상태 저장 (FIFO 리스트)
previous_red_detected = False  # 초기값 설정
DETECTION_HISTORY = 10  # 최근 프레임 수 (노이즈 필터링에 사용)
DETECTION_THRESHOLD = 7  # True로 간주할 최소 감지 수 (노이즈 필터링에 사용)

# 메인 루프
while True:
    if not result_queue.empty():
        frame, results = result_queue.get()
        detections = results.pandas().xyxy[0]  # Pandas DataFrame으로 결과 가져오기

        red_detected_in_frame = False  # 현재 프레임에서의 상태 초기화

        for _, row in detections.iterrows():
            x1, y1, x2, y2 = int(row['xmin']), int(row['ymin']), int(row['xmax']), int(row['ymax'])
            conf, cls = row['confidence'], int(row['class'])
            label = f"{classes[cls]} {conf:.2f}"

            if classes[cls] == 'red':
                red_detected_in_frame = True  # 빨간 불 감지
                # 바운딩 박스 및 텍스트 표시
                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 0, 255), 2)  # 사각형 빨간색
                cv2.putText(frame, label, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)

        # 최근 상태 업데이트 (노이즈 필터링 적용)
        recent_detections.append(red_detected_in_frame)  # 현재 프레임 감지 상태 추가
        if len(recent_detections) > DETECTION_HISTORY:  # 최대 히스토리 크기 초과 시 제거
            recent_detections.pop(0)

        # 최근 감지된 True의 개수에 따라 현재 상태 결정 (노이즈 필터링 로직)
        red_detected = recent_detections.count(True) >= DETECTION_THRESHOLD
        # 노이즈로 인해 일시적으로 True/False로 변경되지 않도록 최소 감지 개수를 조건으로 설정

        # 상태가 변경된 경우에만 IPC로 데이터 전송
        if red_detected != previous_red_detected:
            send_to_ipc(fd, red_detected)
            if red_detected:
                print(f"Red light detected. Sending True to IPC.")
            else:
                print(f"No red light detected. Sending False to IPC.")

        # 이전 상태 업데이트
        previous_red_detected = red_detected

        # 화면 출력
        cv2.imshow("YOLOv5 Detection", frame)

    # 'q' 키로 종료
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()

