CC = g++

KOEI_VIEWER = koei_viewer
PALETTE_GEN = palette_gen
LS11_DECODER = ls11_decoder

LDFLAGS = -lSDL2

all: $(KOEI_VIEWER) $(PALETTE_GEN) $(LS11_DECODER)

$(KOEI_VIEWER): koei_viewer.cpp koei_image.cpp
	$(CC) -o $(KOEI_VIEWER) koei_viewer.cpp koei_image.cpp $(LDFLAGS)

$(PALETTE_GEN): palette_gen.cpp
	$(CC) -o $(PALETTE_GEN) palette_gen.cpp $(LDFLAGS)

$(LS11_DECODER): ls11_decoder.cpp
	$(CC) -o $(LS11_DECODER) ls11_decoder.cpp $(LDFLAGS)

clean:
	@find . -name '*.o' -type f -delete
	@find . -name '$(KOEI_VIEWER)' -type f -delete
	@find . -name '$(PALETTE_GEN)' -type f -delete
	@find . -name '$(LS11_DECODER)' -type f -delete

