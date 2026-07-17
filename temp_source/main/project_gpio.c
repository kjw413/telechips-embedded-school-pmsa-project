#include <linux/project_gpio.h>
#include <vdso/bits.h>

int project_tcc_gpio_direction_output(struct gpio_chip *chip, unsigned int gpio,
					    int value)
{
	return pinctrl_gpio_direction_output(chip->base + gpio);
}

void project_tcc_gpio_set(struct gpio_chip *gc, unsigned int gpio, int val)
{
	struct tcc_gpio_port *port = gpiochip_get_data(gc);
	struct tcc_gpio_group *gpio_gr = port->gpio_gr;
	unsigned long bit = BIT(gpio);
	void __iomem *reg;
	int i;

	for (i = 0; i < port->gpio_num_gr; i++) {
		if (!strcmp(gc->label, gpio_gr->name))
			break;
		gpio_gr++;
	}

	reg = port->base + gpio_gr->reg_offset;

	//pr_debug("%s: base : 0x%px, offset 0x%x\n", __func__,
	//port->base, gpio_gr->reg_offset);

	if (val)
		writel_relaxed(bit, reg + GPIO_BIT_SET);
	else
		writel_relaxed(bit, reg + GPIO_BIT_CLEAR);
}