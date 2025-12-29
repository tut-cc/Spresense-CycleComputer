# Spresense/Arduino用Makefile

# 設定
SPRESENSE_FQBN ?= SPRESENSE:spresense:spresense
ARDUINO_FQBN ?= arduino:avr:uno
PORT ?= /dev/ttyACM0
SKETCH ?= .
ARDUINO_CLI ?= arduino-cli

# デフォルトFQBN
FQBN ?= $(SPRESENSE_FQBN)

.PHONY: all spresense arduino compile upload upload_spresense upload_arduino clean help

all: spresense

spresense: FQBN=$(SPRESENSE_FQBN)
spresense: compile

arduino: FQBN=$(ARDUINO_FQBN)
arduino: compile

help: # ヘルプ表示
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  spresense        Spresense向けにコンパイル (デフォルト)"
	@echo "  arduino          Arduino向けにコンパイル (FQBN=$(ARDUINO_FQBN))"
	@echo "  compile          現在のFQBN設定でコンパイル"
	@echo "  upload_spresense Spresenseに書き込み"
	@echo "  upload_arduino   Arduinoに書き込み"
	@echo "  upload           現在のFQBN設定で書き込み (usage: make upload PORT=/dev/ttyUSB0)"
	@echo "  clean            ビルドキャッシュのクリーン"
	@echo "  help             ヘルプ表示"

compile: 
	$(ARDUINO_CLI) compile --fqbn $(FQBN) $(SKETCH)

upload_spresense: FQBN=$(SPRESENSE_FQBN)
upload_spresense: compile upload

upload_arduino: FQBN=$(ARDUINO_FQBN)
upload_arduino: compile upload

upload: # スケッチをマイコンボードに書き込む
	$(ARDUINO_CLI) upload -p $(PORT) --fqbn $(FQBN) $(SKETCH)

clean:
	$(ARDUINO_CLI) cache clean
