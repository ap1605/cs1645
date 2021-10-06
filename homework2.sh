#!/bin/sh
	for i in {1..150}
	do
		./trap_parallel
	done
	clear
	clear
	for i in {1..150}
	do		
		./mandelbrot_parallel
	done
	clear
	clear
	for i in {1..150}
	do
		./matrix_parallel
	done
