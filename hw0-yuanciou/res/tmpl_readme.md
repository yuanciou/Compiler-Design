# HW0

|        |                  |
| :----: | :--------------: |
|  Name  |     ${name}      |
|   ID   |      ${id}       |
| GitHub | ${github_handle} |
| Email  |     ${email}     |

> **Please revise `student_info.ini` and type `make` to automatically update README.md.**

- Last Make: `${last_maketime}`

- Docker ENV FLAG: `${docker_env_tag}`

---

You have four goals in this assignment:

1. Make sure the environment is set properly so that you can use this environment for future assignments.
2. Know how to use Docker.
3. Know how to GNU Make.
4. Know how to use Git to manage your codes and to submit your assignment by pushing your codes to GitHub.

---

Table of Content

- [Tools that are used in this course](#tools-that-are-used-in-this-course)
  - [Docker](#docker)
  - [Git](#git)
  - [Make](#make)
- [Workflow of HW0](#workflow-of-hw0)
  - [Step 1: Installing Docker](#step-1-installing-docker)
  - [Step 2: Installing Git](#step-2-installing-git)
  - [Step 3: Using `git clone` to clone your HW0 repository](#step-3-using-git-clone-to-clone-your-hw0-repository)
  - [Step 4: Getting the HW0 docker image](#step-4-getting-the-hw0-docker-image)
  - [Step 5: Filling in correct information and using `Make`](#step-5-filling-in-correct-information-and-using-make)
  - [Step 6: Using Git to push your homework to Github](#step-6-using-git-to-push-your-homework-to-github)
- [Important notice](#important-notice)
- [Miscellaneous](#miscellaneous)

---

# Tools that are used in this course

## Docker

For all assignments in this course, you will develop and run your codes in a virtualized environment so as to ensure that everyone works in the same environment and to avoid issues that are unique to a specific platform.

When it comes to virtualization, [VirtualBox](https://www.virtualbox.org/) might be a product you might think of. VirtualBox is a virtual machine, or hypervisor, for x86 virtualization. It virtualizes almost any hardware of the host and allows users to run almost any operating system on a single machine. However, hypervisor-level virtualization introduces performance overheads. Instead of virtualizing the underlying hardware, containers virtualize the operating system so each individual container contains only the application and its libraries and dependencies. Containers are small, fast, and portable because, unlike a virtual machine, containers do not need to include a guest OS in every instance and can, instead, simply leverage the features and resources of the host OS.

In this course, we will use [Docker](https://www.docker.com/), a popular package for building and deploying containers, to build a development environment for future assignments.

In order to ease your burden, we have **wrapped all the commands for setting up Docker**. You just need to understand and follow the workflow. Here is some basic information that you should know about Docker.

### Terminology

- Docker image:

  An image represents a set of files which will run as a container in their own memory and disk and user space. You will be given an image that contains the environment you will develop in.

- Docker container:

  A Docker container is a popular lightweight, standalone, executable container that includes everything needed to run an application, including libraries, system tools, code, and runtime.

### Supported platforms for running Docker

You can run both Linux and Windows programs and executables in Docker containers. The Docker platform runs natively on Linux (on x86-64, ARM and many other CPU architectures) and on Windows (x86-64).

## Git

Are you familiar with the situation in the comics below?

![comics](./res/imgs/version_control_motivation_comics.png)

Source: "Piled Higher and Deeper" by Jorge Cham, http://www.phdcomics.com

We've all been in this situation before; it seems ridiculous to have multiple nearly identical versions of the same document. Some word processors let us deal with this situation a little better, such as with Microsoft Word's "Track Changes" feature; however, it can be quite impractical if you would like to see changes that are older than your previous round of changes.

Version control is used to track and store changes in your files without losing the history of your past changes.

Version control systems start with a base version of the document and then save just the changes you make at each step of the way. You can think of it as a tape: if you rewind the tape and start at the base document, then you can play back each change and end up with your latest version.

![Illustration of committing changes](./res/imgs/play-changes.png)

A version control system is a tool that keeps track of these changes for us and helps us version our files. It allows you to decide which changes make up the next version, called a commit, and keeps useful metadata about them. The complete history of commits for a particular project and their metadata make up a repository (such as our course material repositories). Repositories can be kept in sync across different computers facilitating also collaboration among different people.

One of the most obvious reasons to use version control is to avoid the situation illustrated in the comics above (i.e., to **keep track of the complete history of your changes in a systematic way** without the need to have multiple versions of the same file.) One handy feature of version control is the ability to "go back in time" (i.e., **if something goes wrong, you can start from some earlier version of the file when everything was still working.**) You can also compare the differences between versions and see what has changed. In addition to the aforementioned aspects, version control makes it possible for multiple people to work on the same file or project at the same time while still keeping track of their own changes to the files.

There are multiple different Version Control Systems (VCS) (i.e., software for doing version control), but one of the most popular is Git.

![Git logo](./res/imgs/coward_savior.jpg)

For more details about Git, please refer to [Pro Git](https://git-scm.com/book/en/v2) or [為你自己學 Git](https://gitbook.tw/chapters/using-git/init-repository.html).

## Make

**(GNU Make)**

In this course, we don't write a compiler with bare hands. Instead we use tools, such as [flex](https://github.com/westes/flex) and [bison](https://www.gnu.org/software/bison/), to help us to build a compiler. These tools produce C codes from what you write, and then we use `make` to build up the whole compiler program.

In a project, we usually deal with lots of files, which might depend with each other. We will use `GNU Make`, to help manage the project. Using Make is easy, just execute `make <target>` under the project directory. Then, Make will look for a text file named "Makefile" or "makefile" (with no filename extension) and perform the commands according to the content of `<target>`.

More information about Make is available at [Official Documents](https://www.gnu.org/software/make/manual/make.html) or [Make 命令教學](http://www.ruanyifeng.com/blog/2015/02/make.html) written by 阮一峰.

# Workflow of HW0

## Step 1: Installing Docker

<details>
<summary> Windows (Click to expand!)</summary>

- Method 1: Windows Subsystem for Linux 2 (WSL 2)

WSL 2 provides a Linux kernel, which enables Linux containers to execute directly on Windows without a simulator.
Below are prerequisites to enable this functionality and support the usage of Docker:

1. Install Windows 10 (version 2004 or higher).

2. Enable WSL 2 on Windows. Please refer to _Install the Windows Subsystem for Linux_ and _Update to WSL 2_ in [Microsoft documentation](https://docs.microsoft.com/en-us/windows/wsl/install-win10).

3. Download and install [Linux kernel update package](https://docs.microsoft.com/en-us/windows/wsl/wsl2-kernel).

Now you may install the Linux distribution you prefer with WSL 2. Please refer to _Install your Linux distribution of choice_ in [Microsoft documentation](https://docs.microsoft.com/en-us/windows/wsl/install-win10) for more details.

After that, you may install [Docker Desktop](https://hub.docker.com/editions/community/docker-ce-desktop-windows/).

Finally, you need to allow Docker to use the kernel of WSL 2. Please refer to [Docker Desktop WSL 2 backend](https://docs.docker.com/docker-for-windows/wsl/) for more details.

> Note: If you're using WSL, be sure to check out the [miscellaneous](#wsl) issues related to it.

- Method 2: Virtual Machine

Start a [VMware Workstation Player](https://www.vmware.com/products/workstation-player/workstation-player-evaluation.html) and install [the Ubuntu 18.04](https://ubuntu.com/download/server/thank-you?country=TW&version=18.04.4) image. Do not install Docker on Window directly since we will use some Unix tools in the future assignments. You may refer to [this article](https://www.digitalocean.com/community/tutorials/how-to-install-and-use-docker-on-ubuntu-18-04) for how to install Docker on Ubuntu 18.04.

After entering the Ubuntu Linux, execute the following commands so as to get rid of using `sudo` to execute Docker every time.

```shell
sudo groupadd docker
sudo gpasswd -a $USER docker
sudo service docker restart
```

</details>

<details>
<summary> Linux (Click to expand!)</summary>

Linux users can refer to [Docker document](https://docs.docker.com/install/linux/docker-ce/centos/). Choose the Linux distribution in the menu on the left.

Execute the following commands so as to get rid of using `sudo` to execute Docker every time.

```sh
sudo groupadd docker
sudo gpasswd -a $USER docker
sudo service docker restart
```

</details>

<details>
<summary> macOS (Click to expand!)</summary>

Please refer to [Docker document](https://docs.docker.com/docker-for-mac/install/) to download and install Docker.

</details>

## Step 2: Installing Git

<details> <summary>Windows</summary>

Entering Ubuntu VM or open WSL2, and then

```shell
apt-get install git
```

</details>

<details> <summary>Ubuntu (Linux)</summary>

```shell
apt-get install git
```

</details>

<details> <summary>OSX</summary>

You are recommended to install [brew](https://brew.sh/), which is a package management tool on OSX.

```shell
brew install git
```

After the installation, execute `git --version` to verify whether the installation is correct.

```shell
git --version
```

</details>

### Setting up Git/GitHub Account

> [!note]
> This setting is optional, but we strongly suggest you complete it.

GitHub uses the email address in the commit header to link the commit to a GitHub user, so if you don't setup an email, GitHub cannot link your commit with your account. This will somewhat make your commit unrecognizable. You may refer to
[Why are my commits linked to the wrong user](https://help.github.com/en/articles/why-are-my-commits-linked-to-the-wrong-user).

If you want GitHub to know who made a commit, you need to setup your email address properly (in fact, only setting up email addresses still cannot specify who did the commit, you can refer to [git-blame-someone-else](https://github.com/jayphelps/git-blame-someone-else)):

Please use the following commands to change your Git configuration settings (**use your email address**):

```shell
git config --global user.name your_github_id
git config --global user.email "email@example.com"
```

Then, verify whether the setting is done correctly:

```shell
git config --global user.email
email@example.com
```

The last thing is to set your email address in your GitHub Account settings. See: [Setting your commit email address on GitHub](https://help.github.com/en/articles/setting-your-commit-email-address#setting-your-commit-email-address-on-github).

From now on, the commits you made on your computer will be associated with the email address.

### Creating a personal access token

Since [GitHub no longer allows using passwords for identity verification](https://github.blog/2020-12-15-token-authentication-requirements-for-git-operations), we need to use an [SSH key](https://docs.github.com/en/authentication/connecting-to-github-with-ssh) or [create a personal access token](https://docs.github.com/en/github/authenticating-to-github/keeping-your-account-and-data-secure/creating-a-personal-access-token).

If you opt for a token, be sure to copy it and keep it in a safe place. **You'll need it to verify your identity when executing Git commands via HTTPS, eliminating the need for password input**.

> [!note]
> You may also store the username and password for future operations, which is not necessary.
>
> ```sh
> git config --global credential.helper store
> ```

## Step 3: Using `git clone` to clone your HW0 repository

Enter the repository you want to clone on GitHub and then find the URL for cloning. The steps are as follows, along with the annotated image.

![clone example](./res/imgs/clone_example.jpg)

1. Click the **Code** button.
2. Select the corresponding URL.
   - If you're using an access token, select **HTTPS**. The URL should start with `https://`.
   - If you're using an SSH key, select **SSH**. The URL should start with `git@`.
3. Copy the URL.
4. Execute the following command in your terminal:
   ```shell
   git clone <the repo's URL>
   ```
   If you're using an access token, you'll need to enter your GitHub username and use the access token as the password.

After completing these steps, you'll have a local copy of your HW0 repository in your working directory.

## Step 4: Getting the HW0 Docker image

Execute command `make docker-pull` under the HW0 repository that you just cloned in order to get the HW0 Docker image.

```shell
$ make docker-pull

docker pull laiyt/compiler-f23-hw0:latest
latest: Pulling from laiyt/compiler-f23-hw0
Digest: sha256:5ee9ee3dee63f5bb92d788517eb8fbaebe706392de7d5f659e43c48df22ce14b
Status: Image is up to date for laiyt/compiler-f23-hw0:latest
docker.io/laiyt/compiler-f23-hw0:latest
```

Use the following command to check if the image is downloaded successfully:

```shell
$ docker image ls | grep hw0

laiyt/compiler-f23-hw0              latest    9cd25f612e02   8 days ago      128MB
```

> If you encounter errors, please check if the Docker daemon is started.

**You need to execute `make docker-pull` to get the corresponding image for each assignment.**

Next, execute `./activate_docker.sh` to activate the environment (with the Docker image downloaded) for doing your assignment. (The path to your repository shouldn't contain any white space!)

```shell
$ ./activate_docker.sh
# Entering Docker
```

The space inside Docker is actually separated from your OS. However, when executing `./activate_docker.sh`, the current working directory will be mapped to the home directory in Docker so as to fulfill the need of the assignments.

```shell
# Inside Docker
$ ls
# You will see the same files as outside Docker
```

At this time, the directories inside the local repository are shared with Docker.

We suggest you write your assignment in this way:

- Open a terminal for running Docker. All compilations are done under the Docker environment.
- Use the editor that you are familiar with to write the assignment under your host system. All changes you make are mapped to the Docker environment instantly.

## Step 5: Filling in correct information and using `Make`

### Revise `student_info.ini` to update `README.md`

**What is README.md?**

Every tool or project should have its own documents or manual. When browsing a repository (or a directory) on GitHub, the GitHub website will render the content of the `README.md` file under the repository (or the directory) by default. The filename extension `md` indicates that the file is written in Markdown style. Markdown is a language that has simple syntax and widely used for documentation. This HW0 description and the course website were all written in Markdown. **You will also be asked to write reports in Markdown in the future assignments**. So, it is worth the effort to spend some time to learn about how to write in Markdown.

We recommend you to practice on [HackMD](https://hackmd.io/). There are also some simple [instructions](https://hackmd.io/getting-started). Notice that HackMD is a company that was founded by NTUT alumni, so it supports Chinese better than other online editors.

**Hands-on**

In this assignment, you need to revise `student_info.ini` by filling in your personal information. Then, execute `make` to update the README.md file under the repository. Don't worry about privacy issues. Each repository you obtain from the course organization will be visible to only you and TAs.

Here are the fields that you need to change in `student_info.ini`:

- name: `your name`
- id: `your student id`
- github_handle: `your GitHub username`
- email: `the email address in your GitHub settings`

**An example of `student_info.ini`:**

```ini
[info]
    name = compiler
    id = 0612999
    github_handle = johnny123
    email = johnny123@gmail.com
```

> Please fill in your personal information.

After filling in the correct information in `student_info.ini`, please execute the `make` command in the home directory of your repository **under the Docker environment**. Then, check if the content of README.md has been changed to your information.

> It will show errors if you do not execute `make` under the Docker environment. This is to make sure you know how to complete this assignment inside Docker.

![README Revision](./res/imgs/make_readme.jpg)

The benefit of using Make is that users don't need to understand how the project is built. If you don't know what to do, just type `make` and everything will be done. Sometimes you need to read the description inside a Makefile first and make some actions accordingly. Please notice that **you will use a Makefile to build your project in future assignments**.

### Review `README.md`

After finishing the previous procedure, the following two fields in README.md will also be updated:

- Last Make: `unknown`
- Docker ENV FLAG: `unknown`

If you do the previous procedure correctly, the last compilation time and `DOCKER_ACTIVATED` will be shown here.

> You can use `make clean` to restore your project back to the initial state.

## Step 6: Using Git to push your homework to GitHub

### Turning in your homework

When you reach this step, `student_info.ini` and `README.md` have been changed. The next thing to do is to make these revisions as a new version under Git's control.

You may first use `git status` to see the current status of the repository.

```shell
$ git status

On branch main
Your branch is up to date with 'origin/main'.

Changes not staged for commit:
    (use "git add <file>..." to update what will be committed)
    (use "git checkout -- <file>..." to discard changes in working directory)

    modified:   student_info.ini
    modified:   README.md

no changes added to commit (use "git add" and/or "git commit -a")
```

Type `git add student_info.ini README.md` to add the revised files for a commit. You may also use `git add .` to add all files that are changed after the last commit.

```shell
git add student_info.ini README.md
```

Use `git status` to check the status again.

```shell
$ git status
Your branch is up to date with 'origin/main'.

Changes to be committed:
    (use "git reset HEAD <file>..." to unstage)

    modified:   student_info.ini
    modified:   README.md
```

Use `git commit -m "the commit message"` to make a commit. The `-m` option is used to supplement the log message of the created commit so that you can quickly understand the changes of a commit when you look back the commit history.

```shell
$ git commit -m ":pencil: update student information"

[main 25b95d1] :pencil: update student information
    2 files changed, 8 insertions(+), 8 deletions(-)
```

Next, use `git push` to synchronize your local repository with the remote repository on GitHub.

```shell
$ git push

Enumerating objects: 7, done.
Counting objects: 100% (7/7), done.
Delta compression using up to 4 threads
Compressing objects: 100% (4/4), done.
Writing objects: 100% (4/4), 511 bytes | 255.00 KiB/s, done.
Total 4 (delta 2), reused 0 (delta 0)
remote: Resolving deltas: 100% (2/2), completed with 2 local objects.
To github.com:Compiler-s24/hw0-johnny123.git
    a80fff2..25b95d1  main -> main
```

At this point, you should be able to see the commit on the GitHub repository.

> In this course, we recommend you to make a push once you have made a commit.

# Important notice

- **An assignment must be submitted via `git push`**, and **the submission time is determined by the time of the last push**, not the time of the last commit, of your GitHub repository. Make sure that all the commits within your local repository are pushed to the GitHub repository by the deadline. If you do not intend to be labeled as late submission, **do not push anything after the deadline**.

- All assignments will be graded under the environment (the Docker image) we provide you. No matter what platform you work on, **please make sure your codes can be compiled successfully in the designated way**, which is also provided in the Docker image.

# Miscellaneous

## WSL

- Since WSL is built on the Windows file system, there's an issue with setting the owner and group of files using _chmod_/_chown_ and modifying read/write/execute permissions in WSL on mounted files. To resolve this, navigate to the `/etc/wsl.conf` file in WSL (or create it if it doesn't exist) and add the following section:

  ```conf
  [automount]
  options = metadata
  ```

  Afterward, exit the WSL shell and restart it using the following commands[^1]:

  ```sh
  # Terminate a specific distro:
  $ wsl.exe --terminate <DistroName>
  # Boot up a specific distro:
  $ wsl.exe --distribution <DistroName>
  ```

  Enabling this option ensures that files retain their metadata for WSL compatibility.

  For more details on this issue, please refer to the [Chmod/Chown WSL Improvements](https://devblogs.microsoft.com/commandline/chmod-chown-wsl-improvements/) article.

  [^1]: [Rebooting Ubuntu on Windows without rebooting Windows?](https://superuser.com/a/1347725)

- Windows file system treats file and directory names as case-insensitive, which may lead to compilation errors on Linux, as a case typo on the file name doesn't prevent it from being recognized in Windows, but does in Linux. WSL by default inherits such insensitivity. To force it to be case-sensitive, navigate to the `/etc/wsl.conf` file in WSL and append `case=force` to the options (no whitespace around the comma):

  ```conf
  [automount]
  options = ...,case=force
  ```

  For more details on this issue, please refer to the [Adjust case sensitivity](https://learn.microsoft.com/en-us/windows/wsl/case-sensitivity) article.
