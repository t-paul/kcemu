## Process this file with automake to produce Makefile.in

KCTAPE=../src/libtape/kctape
TAPENAME=files.kct

pkgdata_DATA = \
	files.kct \
	z1013.kct \
	wordpro.kct

EXTRA_DIST = \
	$(pkgdata_DATA)			\
	bennion_rbs_0x0200_0x46c0.prg	\
	boulderdash_0x0400.prg		\
	boulderdash_0x4000.scr

tape:
	rm -f files.kct
	$(KCTAPE) $(TAPENAME) -c
	$(KCTAPE) $(TAPENAME) --add-raw bennion_rbs_0x0200_0x46c0.prg \
			"Bennion RBS" "BENNION" 0x0200 0x46c0
	$(KCTAPE) $(TAPENAME) --add-raw digger_0x0200.prg \
			"Digger" "DIGGER" 0x0200 0x3c3d
	$(KCTAPE) $(TAPENAME) --add-raw boulderdash_0x0400.prg \
			"Boulder Dash" "BOULDER" 0x0400 0x25cc
	$(KCTAPE) $(TAPENAME) --add-raw boulderdash_0x4000.scr \
			"Boulder Dash Screens" "SCREENS" 0x4000 0xffff
	$(KCTAPE) $(TAPENAME) --add-raw flipper_0x0200.prg \
			"Flipper" "FLIPPER" 0x0200 0x020a
	$(KCTAPE) $(TAPENAME) --add-raw invaders_0x0200.prg \
			"Invaders" "INVADER" 0x0200 0x020b
	$(KCTAPE) $(TAPENAME) --add-raw ladder.4_0x10b0.prg \
			"Ladder 4" "KCLADDER" 0x10b0 0x1108
	$(KCTAPE) $(TAPENAME) --add-raw tetris_0x0c1e.prg \
			"Tetris" "TETRIS" 0x0c1e 0x2299
	$(KCTAPE) $(TAPENAME) --add-raw xonix_0x6000.prg \
			"Xonix" "XONIX" 0x6000 0x6064
	$(KCTAPE) $(TAPENAME) --add deliro4.kcc "Deliro 4 Loader"
	$(KCTAPE) $(TAPENAME) --add deliro.kcc "Deliro 4"
