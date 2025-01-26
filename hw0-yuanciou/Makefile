all: update-info

.PHONY: restore update-info clean
clean: restore

IMAGE_NAME = compiler-f23-hw0
DOCKERHUB_HOST_ACCOUNT = laiyt
IMAGE_FULLNAME = ${DOCKERHUB_HOST_ACCOUNT}/${IMAGE_NAME}:latest

update-info:
	python3 src/update-info.py

restore:
	python3 src/update-info.py --restore --config "res/default_info.ini"

docker-pull:
	docker pull ${IMAGE_FULLNAME}

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
		--hostname ${HOST_NAME}
