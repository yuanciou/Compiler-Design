all: build
.PHONY: build

IMAGE_NAME ?= dummy-image

build: Dockerfile
	docker build -t $(IMAGE_NAME) .
