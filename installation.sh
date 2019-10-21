#!/bin/bash

DEPENDENCY_FOLDER="/home/rafaelgss/project_dependencies";

# Update for old repositories.
apt update -y;

# Installing dependencies
apt install -y git libcurl4-openssl-dev;

# Create folder to store external dependencies
mkdir -p $DEPENDENCY_FOLDER;

# Download external dependencies like WPP and Boost 1_58
git clone https://github.com/1ncrivelSistemas/wpp.git $DEPENDENCY_FOLDER/wpp;
git clone https://github.com/1ncrivelSistemas/lib.boost.1_58.git $DEPENDENCY_FOLDER/lib.boost.1_58;

# Setting environment variables
source ./bashrc.sh $DEPENDENCY_FOLDER;

# Extract libboost
cd $DEPENDENCY_FOLDER;
tar xvjf lib.boost.1_58/boost_1_58_0.tar.bz2;

# Set permissions on folder
chmod -R 755 $DEPENDENCY_FOLDER;

