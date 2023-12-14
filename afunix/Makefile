#
# environment variables
#

CC=gcc
CFLAGS=-std=c2x -Wall -Wextra
DEPS=log.h log-hub.h log-pub-test.h
BUILD=./build

#
# build targets
#

.PHONY: all
all: log-hub log-pub log-sub

$(BUILD):
	mkdir -p $(BUILD)

.PHONY: log-hub
log-hub: $(BUILD)/log-hub

.PHONY: log-pub
log-pub: $(BUILD)/log-pub

.PHONY: log-sub
log-sub: $(BUILD)/log-sub

$(BUILD)/%.o: %.c $(DEPS) | $(BUILD)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILD)/log-hub: $(BUILD)/log.o $(BUILD)/log-hub.o | $(BUILD)
	$(CC) -o $(BUILD)/log-hub $(BUILD)/log.o $(BUILD)/log-hub.o $(CFLAGS)

$(BUILD)/log-pub: $(BUILD)/log.o $(BUILD)/log-pub.o | $(BUILD)
	$(CC) -o $(BUILD)/log-pub $(BUILD)/log.o $(BUILD)/log-pub.o $(CFLAGS)

$(BUILD)/log-sub: $(BUILD)/log.o $(BUILD)/log-sub.o | $(BUILD)
	$(CC) -o $(BUILD)/log-sub $(BUILD)/log.o $(BUILD)/log-sub.o $(CFLAGS)

#
# clean targets
#

.PHONY: clean
clean:
	rm -rf $(BUILD)


#
# run targets
#

.PHONY: run-log-hub
run-log-hub: log-hub
	$(BUILD)/log-hub

.PHONY: run-log-pub
run-log-pub: log-pub
	$(BUILD)/log-pub

.PHONY: run-log-sub
run-log-sub: log-sub
	$(BUILD)/log-sub $(filter-out $@,$(MAKECMDGOALS))

