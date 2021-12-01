KOEI_VIEWER = koei_viewer

LDFLAGS = -lsdl2

all: $(KOEI_VIEWER)

$(KOEI_VIEWER): koei_viewer.c koei_image.c
	gcc -o $(KOEI_VIEWER) koei_viewer.c koei_image.c $(LDFLAGS)

clean:
	@find . -name '*.o' -type f -delete
	@find . -name '$(KOEI_VIEWER)' -type f -delete

