.PHONY: clean synth

SRC = $(wildcard src/camera/*.sv src/display/*.sv src/detection/*.sv src/spi/*.sv src/top/*.sv)

%.sim: tb/%_tb.sv $(SRC)
	mkdir -p build
	iverilog -g2012 -o build/$@ $< $(SRC)
	vvp build/$@
	gtkwave build/$*.vcd

synth: $(SRC)
	mkdir -p build
	yosys -p "synth_ecp5 -json build/top.json" $^
	nextpnr-ecp5 --25k --package CABGA256 --speed 6 --json build/top.json --textcfg build/top.cfg --lpf src/constraints/camera.lpf --freq 65
	ecppack --svf build/top.svf build/top.cfg build/top.bit

clean:
	rm -f build/*