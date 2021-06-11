# We used ubuntu 20.10 to make sure the project will have the behaviour we were having on our system.
FROM ubuntu:20.10

# Update the list of files available in the APT repositories.
RUN apt-get update
# Install the package containing 'make' and 'gcc' in order to compile the project (the -y option is here to answer yes to confirmation requests).
RUN apt-get -y install build-essential

# Set the environment variable "HOME", in order to make sure the project will work correctly 
# For exemple: cd without arguments needs this variable to be set.
ENV HOME="/home/user"

# Set the working directory for the instructions that follow in this Dockerfile.
WORKDIR /home/user/project/tsh/

# Add the content of the directory in wich the Dockerfile is located to the working directory (defined just before).
ADD . .

# If you want the project to be compiled with the docker image build, delete the '#' in the line below.
#RUN make
