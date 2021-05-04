FPGA_PACKAGE = tq144
FPGA_DEVICE = hx1k

###

PROJ = picohx_demo
TOP = $(PROJ)_top

ICE40_PIN_DEF = ../data/picohx.pcf

SOURCES := \
	$(TOP).v \
	debouncer.v

VIDEO_SOURCES := $(addprefix video/, \
	vga_timing.v \
	vga_core.v \
	dvi-12bit.v \
	)

SOURCES += $(VIDEO_SOURCES)

###

main: $(PROJ).bin
header: $(PROJ).h

count: $(SOURCES)
	yosys -p 'synth_ice40 -top $(TOP) -noflatten' $^

%.json: $(SOURCES)
	yosys -p 'synth_ice40 -top $(TOP) -json $@' $^

%.asc: $(ICE40_PIN_DEF) %.json
	nextpnr-ice40 --package $(FPGA_PACKAGE) --$(FPGA_DEVICE) --json $(filter-out $<,$^) --placer heap --seed 0 --pcf $< --asc $@

%.bin: %.asc
	icepack $< $@

%.h: %.bin
	xxd -i $^ > $@

###

prog: $(PROJ).bin
	../../scripts/picoprog.py $<

clean:
	rm -f $(PROJ).bin $(PROJ).asc $(PROJ).json $(PROJ).h

.SECONDARY:
.PHONY: main header clean prog

