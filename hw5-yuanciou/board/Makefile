.PHONY: all compile run clean env

all: compile run

compile:
	../src/compiler src/boardTest.p --save_path src/

%.tar.gz:
	wget -nv -O $@ 'https://master.dl.sourceforge.net/project/platformio-storage/packages/$@'

env: toolchain-gd32v-linux_x86_64-9.2.0.tar.gz tool-openocd-gd32v-linux_x86_64-0.1.1.tar.gz tool-gd32vflash-linux_x86_64-0.1.0.tar.gz

run: env
	pio run
	pio run --target upload

clean:
	$(RM) -r src/boardTest.S .pio/
