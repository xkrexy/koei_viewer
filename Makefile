CC = g++

KOEI_VIEWER = koei_viewer
PALETTE_GEN = palette_gen

LDFLAGS = -lSDL2

all: $(KOEI_VIEWER) $(PALETTE_GEN)

$(KOEI_VIEWER): koei_viewer.cpp koei_image.cpp
	$(CC) -o $(KOEI_VIEWER) koei_viewer.cpp koei_image.cpp $(LDFLAGS)

$(PALETTE_GEN): palette_gen.cpp
	$(CC) -o $(PALETTE_GEN) palette_gen.cpp $(LDFLAGS)

clean:
	@find . -name '*.o' -type f -delete
	@find . -name '$(KOEI_VIEWER)' -type f -delete
	@find . -name '$(PALETTE_GEN)' -type f -delete
