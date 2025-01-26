all: project

.PHONY: restore project test clean autograde

IMAGE_NAME = compiler-f23-hw1
DOCKERHUB_HOST_ACCOUNT = laiyt
IMAGE_FULLNAME = ${DOCKERHUB_HOST_ACCOUNT}/${IMAGE_NAME}:latest


# TODO: add a clean build opiton
project:
	${MAKE} -C src/
project-clean:
	${MAKE} clean -C src/

test: project
	${MAKE} -C test/
test-clean:
	${MAKE} clean -C test/

clean: project-clean test-clean

docker-pull:
	docker pull ${IMAGE_FULLNAME}

# NOTE: The test script exits with 1 if any test case fails, causing Make itself to fail by default.
# This results in the error being treated as a compilation error, preventing the `diff` of the
# outputs from being taken, leading to incorrect content in the diff section of the report.
# To address this, `--ignore-errors` is added to Make to allow it to succeed even if the testing process fails.

autograde: clean
	${MAKE} project && ${MAKE} --ignore-errors test

# Docker
# ========================================================
# ██████╗  ██████╗  ██████╗██╗  ██╗███████╗██████╗
# ██╔══██╗██╔═══██╗██╔════╝██║ ██╔╝██╔════╝██╔══██╗
# ██║  ██║██║   ██║██║     █████╔╝ █████╗  ██████╔╝
# ██║  ██║██║   ██║██║     ██╔═██╗ ██╔══╝  ██╔══██╗
# ██████╔╝╚██████╔╝╚██████╗██║  ██╗███████╗██║  ██║
# ╚═════╝  ╚═════╝  ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝
# ========================================================


.PHONY: docker-build docker-push activate

CONTAINER_USERNAME = student

HOST_NAME = compiler-course
HOMEDIR = /home/$(CONTAINER_USERNAME)
BINDDIR = $(HOMEDIR)/hw

# ===================== end Docker args

docker-push: docker-build
	docker login && docker tag ${IMAGE_NAME} ${IMAGE_FULLNAME} && docker push ${IMAGE_FULLNAME}

docker-build:
	${MAKE} \
		IMAGE_NAME=${IMAGE_NAME} \
		-C docker

activate:
	python3 docker/activate_docker.py \
		--username ${CONTAINER_USERNAME} \
		--homedir ${HOMEDIR} \
		--binddir ${BINDDIR} \
		--imagename ${IMAGE_FULLNAME} \
		--hostname ${HOST_NAME} ${ARGS}
