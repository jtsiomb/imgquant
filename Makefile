PREFIX = /usr/local

obj = src/main.o src/image.o src/quant.o src/tiles.o
bin = imgquant

CFLAGS = -pedantic -Wall -Wno-unused-function -g
LDFLAGS = -lpng -lz -lm

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

clean:
	$(RM) src/*.o
	$(RM) imgquant

install: $(bin)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(bin) $(DESTDIR)$(PREFIX)/bin/$(bin)

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(bin)
