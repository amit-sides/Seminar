# set base image (host OS)
FROM python:3.8

# set the working directory in the container
WORKDIR /app

# copy the dependencies file to the working directory
COPY docker/requirements.txt .

# install dependencies
RUN pip install -r requirements.txt

# copy the content of the local src directory to the working directory
COPY docker/ .

# define what ports will be exposed to the host
EXPOSE 8080

# command to run on container start
CMD [ "python", "./runner.py" ]
