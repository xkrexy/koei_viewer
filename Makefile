KOEI_VIEWER = koei_viewer
PALETTE_GEN = palettegen

LDFLAGS = -lSDL2

all: $(KOEI_VIEWER) $(PALETTE_GEN)

$(KOEI_VIEWER): koei_viewer.c koei_image.c
	gcc -o $(KOEI_VIEWER) koei_viewer.c koei_image.c $(LDFLAGS)

$(PALETTE_GEN): palettegen.c
	gcc -o $(PALETTE_GEN) palettegen.c $(LDFLAGS)

clean:
	@find . -name '*.o' -type f -delete
	@find . -name '$(KOEI_VIEWER)' -type f -delete
	@find . -name '$(PALETTE_GEN)' -type f -delete

