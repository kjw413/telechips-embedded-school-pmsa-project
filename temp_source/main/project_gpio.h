#ifndef PROJECT_GPIO_H
#define PROJECT_GPIO_H

#include <linux/gpio.h>

#define GPIO_C23 84
#define GPIO_C24 85
#define GPIO_C28 89

#define GPIO_BIT_SET 0x8
#define GPIO_BIT_CLEAR 0xC

struct tcc_gpio_port;

struct tcc_gpio_group {
	struct tcc_gpio_port *port;
	struct gpio_chip gc;
	struct irq_chip ic;
	u32 source_section;
	u32 *gpio_offset_num;
	u32 *source_offset_num;
	u32 *source_range;
	const char *name;
	u32 reg_offset;
};

struct tcc_gpio_port {
	int gpio_num_gr;
	void __iomem *base;
#if defined(CONFIG_PINCTRL_TCC_SCFW)
	u32 raw_base;
#endif
	struct tcc_gpio_group *gpio_gr;
	const struct tcc_gpio_soc_data *sdata;
	int *irq;
	int irq_num;
	int **irq_port_map;
};


int project_tcc_gpio_direction_output(struct gpio_chip *chip, unsigned int gpio, int value);
void project_tcc_gpio_set(struct gpio_chip *gc, unsigned int gpio, int val);

#endif
