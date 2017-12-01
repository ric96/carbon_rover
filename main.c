/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <gpio.h>
#include <misc/printk.h>
#include <sys_clock.h>
#include <misc/util.h>

#define PORT	"GPIOA"
#define PORT2 "GPIOB"
#define PORT3 "GPIOC"
// Motor Controller Pins GPIOA
#define A1A	0
#define A1B	2
#define B1A	3
#define B1B	1

// IR Edge sensor Pins GPIOB
#define IRFL	12
#define IRFR	15
#define IRBL	14
#define IRBR	13

// Ultrasonic Sensor Pins
#define TRIGFL	2 //GPIOC
#define ECHOFL	4
#define TRIGFC	3 //GPIOC
#define ECHOFC	5
#define TRIGFR	6 //GPIOC
#define ECHOFR	8
#define TRIGBR	7 //GPIOC
#define ECHOBR	9
#define TRIGBC	6 //GPIOB
#define ECHOBC	7
#define TRIGBL	8 //GPIOB
#define ECHOBL	9

#define SLEEP_TIME 1000

#define STACKSIZE 1024
#define PRIORITY 7

#define DELAY K_MSEC(10)

static struct device *gpioa;
static struct device *gpiob;
static struct device *gpioc;

static uint32_t dir = 0, irfr = 0, irfl = 0, irbr = 0, irbl = 0;

static uint32_t us_fl = 0, us_fc = 0, us_fr = 0, us_br = 0, us_bc = 0, us_bl = 0;

void fwd();
void bwd();
void stop();
void fwd_left();
void fwd_right();
void bwd_left();
void bwd_right();
void left();
void right();
uint32_t get_us(uint32_t trig, uint32_t echo, struct device *dev);

void main(void)
{
	gpioa = device_get_binding(PORT);
	gpiob = device_get_binding(PORT2);
	gpioc = device_get_binding(PORT3);

	//Motor
	gpio_pin_configure(gpioa, A1A, GPIO_DIR_OUT);
	gpio_pin_configure(gpioa, A1B, GPIO_DIR_OUT);
	gpio_pin_configure(gpioa, B1A, GPIO_DIR_OUT);
	gpio_pin_configure(gpioa, B1B, GPIO_DIR_OUT);

	//IR
	gpio_pin_configure(gpiob, IRFL, GPIO_DIR_IN);
	gpio_pin_configure(gpiob, IRFR, GPIO_DIR_IN);
	gpio_pin_configure(gpiob, IRBL, GPIO_DIR_IN);
	gpio_pin_configure(gpiob, IRBR, GPIO_DIR_IN);

	//US
	gpio_pin_configure(gpioc, TRIGFL, GPIO_DIR_OUT);
	gpio_pin_configure(gpioc, ECHOFL, GPIO_DIR_IN);
	gpio_pin_configure(gpioc, TRIGFC, GPIO_DIR_OUT);
	gpio_pin_configure(gpioc, ECHOFC, GPIO_DIR_IN);
	gpio_pin_configure(gpioc, TRIGFR, GPIO_DIR_OUT);
	gpio_pin_configure(gpioc, ECHOFR, GPIO_DIR_IN);
	gpio_pin_configure(gpioc, TRIGBR, GPIO_DIR_OUT);
	gpio_pin_configure(gpioc, ECHOBR, GPIO_DIR_IN);
	gpio_pin_configure(gpiob, TRIGBC, GPIO_DIR_OUT);
	gpio_pin_configure(gpiob, ECHOBC, GPIO_DIR_IN);
	gpio_pin_configure(gpiob, TRIGBL, GPIO_DIR_OUT);
	gpio_pin_configure(gpiob, ECHOBL, GPIO_DIR_IN);
}
void fwd()
{
	gpio_pin_write(gpioa, A1A, 1);
	gpio_pin_write(gpioa, A1B, 0);
	gpio_pin_write(gpioa, B1A, 1);
	gpio_pin_write(gpioa, B1B, 0);
	printk("forward\n");
}

void bwd()
{
	gpio_pin_write(gpioa, A1A, 0);
	gpio_pin_write(gpioa, A1B, 1);
	gpio_pin_write(gpioa, B1A, 0);
	gpio_pin_write(gpioa, B1B, 1);
	printk("backward\n");
}

void stop()
{
	gpio_pin_write(gpioa, A1A, 0);
	gpio_pin_write(gpioa, A1B, 0);
	gpio_pin_write(gpioa, B1A, 0);
	gpio_pin_write(gpioa, B1B, 0);
	printk("STOP\n");
}

void fwd_left()
{
	gpio_pin_write(gpioa, A1A, 1);
	gpio_pin_write(gpioa, A1B, 0);
	gpio_pin_write(gpioa, B1A, 0);
	gpio_pin_write(gpioa, B1B, 0);
	printk("forward right\n");
}

void fwd_right()
{
	gpio_pin_write(gpioa, A1A, 0);
	gpio_pin_write(gpioa, A1B, 0);
	gpio_pin_write(gpioa, B1A, 1);
	gpio_pin_write(gpioa, B1B, 0);
	printk("forward left\n");
}

void bwd_left(/* arguments */)
{
	gpio_pin_write(gpioa, A1A, 0);
	gpio_pin_write(gpioa, A1B, 1);
	gpio_pin_write(gpioa, B1A, 0);
	gpio_pin_write(gpioa, B1B, 0);
	printk("back left\n");
}

void bwd_right(/* arguments */) {
	gpio_pin_write(gpioa, A1A, 0);
	gpio_pin_write(gpioa, A1B, 0);
	gpio_pin_write(gpioa, B1A, 0);
	gpio_pin_write(gpioa, B1B, 1);
	printk("back right\n");
}

void left(/* arguments */) {
	gpio_pin_write(gpioa, A1A, 1);
	gpio_pin_write(gpioa, A1B, 0);
	gpio_pin_write(gpioa, B1A, 0);
	gpio_pin_write(gpioa, B1B, 1);
	printk("left spin\n");
}

void right(/* arguments */) {
	gpio_pin_write(gpioa, A1A, 0);
	gpio_pin_write(gpioa, A1B, 1);
	gpio_pin_write(gpioa, B1A, 1);
	gpio_pin_write(gpioa, B1B, 0);
	printk("right spin\n");
}

uint32_t get_us(uint32_t trig, uint32_t echo, struct device *dev)
{
	uint32_t cycles_spent;
	uint32_t nanseconds_spent;
	uint32_t val;
	uint32_t cm;
	uint32_t stop_time;
	uint32_t start_time;
	gpio_pin_write(dev, trig, 1);
	k_sleep(K_MSEC(10));
	gpio_pin_write(dev, trig, 0);
	do {
		gpio_pin_read(dev, echo, &val);
	} while (val == 0);
	start_time = k_cycle_get_32();

	do {
		gpio_pin_read(dev, echo, &val);
		stop_time = k_cycle_get_32();
		cycles_spent = stop_time - start_time;
		if (cycles_spent > 1266720) //260cm for 84MHz (((MAX_RANGE * 58000) / 1000000000) * (CLOCK * 1000000))
		{
			break;
		}
	} while (val == 1);
	nanseconds_spent = SYS_CLOCK_HW_CYCLES_TO_NS(cycles_spent);
	cm = nanseconds_spent / 58000;
	//printk("%d\n", cm);
	return cm;
}

void read_ir()
{
	while(1)
	{
		gpio_pin_read(gpiob, IRFR, &irfr);
		gpio_pin_read(gpiob, IRFL, &irfl);
		gpio_pin_read(gpiob, IRBR, &irbr);
		gpio_pin_read(gpiob, IRBL, &irbl);
		printk("Front Right IR: %d\nFront Left IR: %d\nBack Right IR: %d\nBack Letf IR: %d\n", irfr, irfl, irbr, irbl);
		k_sleep(DELAY);
	}
}

void read_us()
{
	while(1)
	{
		us_fl = get_us(TRIGFL, ECHOFL, gpioc);
		us_fc = get_us(TRIGFC, ECHOFC, gpioc);
		us_fr = get_us(TRIGFR, ECHOFR, gpioc);
		us_br = get_us(TRIGBR, ECHOBR, gpioc);
		us_bc = get_us(TRIGBC, ECHOBC, gpiob);
		us_bl = get_us(TRIGBL, ECHOBL, gpiob);
		printk("Front Left US: %d\nFront Center US: %d\nFront Right IR: %d\nBack Right IR: %d\nBack Center US: %d\nBack Letf IR: %d\n", us_fl, us_fc, us_fr, us_br, us_bc, us_bl);
		k_sleep(DELAY);
	}
}

void run()
{
	while (1) {

		if(irfr == 0 && irfl == 0 && irbr == 0 && irbl == 0)
		{
			stop();
			dir = 0;
		}

		else if(dir == 0)
		{
			if(irfr == 0 && irfl == 1)
			{
				left();
			}

			else if(irfl == 0 && irfr == 1)
			{
				right();
			}

			if (irfr == 1 && irfl == 1)
			{
				fwd();
			}

			else if(irfr == 0 && irfl == 0)
			{
				bwd();
				dir = 1;
			}
		}

		else if(dir == 1)
		{
			if(irbr == 0 && irbl == 1)
			{
				right();
			}

			else if(irbl == 0 && irbr == 1)
			{
				left();
			}

			else if(irbr == 1 && irbl == 1)
			{
				bwd();
			}

			else if(irbr == 0 && irbr == 0)
			{
				fwd();
				dir = 0;
			}
		}

		k_sleep(DELAY);

/*		fwd();
		k_sleep(SLEEP_TIME);
		bwd();
		k_sleep(SLEEP_TIME);
		stop();
		k_sleep(SLEEP_TIME);
		fwd_right();
		k_sleep(SLEEP_TIME);
		fwd_left();
		k_sleep(SLEEP_TIME);
		bwd_right();
		k_sleep(SLEEP_TIME);
		bwd_left();
		k_sleep(SLEEP_TIME);
		left();
		k_sleep(SLEEP_TIME);
		right();
		k_sleep(SLEEP_TIME); */
	}
}

//K_THREAD_DEFINE(run_id, STACKSIZE, run, NULL, NULL, NULL,
	//	PRIORITY, 0, K_NO_WAIT);
//K_THREAD_DEFINE(read_ir_id, STACKSIZE, read_ir, NULL, NULL, NULL,
		//PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(read_us_id, STACKSIZE, read_us, NULL, NULL, NULL,
				PRIORITY, 0, K_NO_WAIT);
