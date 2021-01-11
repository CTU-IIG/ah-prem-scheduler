# C++ simplyfied arrowhead library and sample implementation

**Does not use orchestrator for data exchange**

This repository contains the following:
* ArrowheadLibrary - C++ library for easier implementation of AH producers/consumers
* SchedulerServer - Sample provider implentation - produces schedule: waits for file sent over TCP and generates something to be sent back (actually not only provider)
* CompilerClient - Sample consumer implementation - consumes schedule: Sends file over TCP and waits for schedule (also not only consumer)
* Helper - python scripts that registers consumer and allows consumer to see producer
* Example scripts how to generate custom certificates (copy into our main arrowhead repository)

For both producer and consumer, you have to generate/reuse certificates with the same system name and certificate name. (e.g. you generate certificate with name "alois", then your consumer or producer has to have system name "alois")

Steps in short:
* Compile library, provider and consumer app
* Edit config for provider. Run provider and get output of the registration process
* Fill config for helper script, run helper to register the consumer system and to connect the consumer with the provider
* Edit config for consumer, run consumer.
* Everything works as expected :)

## AH library

Build by using:
    mkdir build && cd build;
    cmake ..
    make

## Scheduler server

Build by using:
    mkdir build && cd build;
    cmake ..
    make
    cd ..
    build/SchedulerServer

Before use, check certificates directory, update json files with configurations. File certificates.json contains paths to required certificates, producer.json contains information about the server (address, port) where the compiler client can send file to solve. System name has to be the same as the client certificate name.

Run:
    cd ..
    build/SchedulerServer

## Compiler client

Build by using:
    mkdir build && cd build;
    cmake ..
    make
    cd ..
    build/CompilerClient

Again, check json files with configurations. File consumer.json contains information about requested service (fill it according to your provider config). Client certificate name has to be the same as requester system name.

## Helper

First, run provider (scheduler server) and check output of the command line. If everything works correctly, then you should get a few JSON lines that you can copy paste into helper script config.

Update json configurations according to the consumer and provider and run. If you already registered consumer system, you can fill its id in python script, otherwise, the post request will fail.

    python3 add_consumer_and_connect.py

If you want to delete or modify records in AH database, you have to do it manually from web ui. :(

## Web ui for checking state and progress

Add into firefox/chromium custom certificate (sysop.p12). Then connect to the AH server:

* https://10.35.127.242:8443 for service registry (to check if provider and consumer are registered correctly) - useful is GET /serviceregistry/mgmt and GET /serviceregistry/mgmt/systems
* https://10.35.127.242:8445 for authorization system (to check if consumer can see the provider) - useful is GET /authorization/mgmt/intracloud

