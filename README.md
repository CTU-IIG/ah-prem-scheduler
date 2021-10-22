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

Before use, check certificates directory, update json files with configurations. File certificates.json contains paths to required certificates, producer.json contains information about the server (address, port) where the compiler client can send file to solve. System name has to be the same as the client certificate name.

Run:

    cd ..
    build/SchedulerServer
    
If producer registers successfully to the AH server, you should see similar lines like those in stdout:

    "providerInterfaceId" : 7,
    "providerId" : 42,
    "providerServiceDefinitionId" : 24

## Helper

First, run provider (scheduler server) and check output of the command line. If everything works correctly, then you should get a few JSON lines that you can copy paste into helper script config.

Update json configurations according to the consumer and provider and run. If you already registered consumer system, you can fill its id in python script, otherwise, the post request will fail.

    python3 add_consumer_and_connect.py

If you want to delete or modify records in AH database, you have to do it manually from web ui. (Shortly described later.) :(

## Compiler client

Build by using:

    mkdir build && cd build;
    cmake ..
    make
    cd ..
    build/CompilerClient

Again, check json files with configurations. File consumer.json contains information about requested service (fill it according to your provider config). Client certificate name has to be the same as requester system name.

## Web ui for checking state and progress

Add into firefox/chromium custom certificate (sysop.p12). Then connect to the AH server: (10.35.127.242 is IP of our AH server)

* https://10.35.127.242:8443 for service registry (to check if provider and consumer are registered correctly) - useful is GET /serviceregistry/mgmt and GET /serviceregistry/mgmt/systems
* https://10.35.127.242:8445 for authorization system (to check if consumer can see the provider) - useful is GET /authorization/mgmt/intracloud

## Other

### Setup AH Services

See [core-java-spring README](https://github.com/arrowhead-f/core-java-spring/blob/master/README.md).

### Generate certificates

Use [Certificate Generation](https://github.com/eclipse-arrowhead/core-java-spring/tree/master/scripts/certificate_generation) from the offical repo.

In here, adjust names and IP addresses to match your setup. Basically, all you need to modify are `ip:[^,]*` and `[^"]*\.ltu\.arrowhead.eu`.

Also, add your password to `lib_certs`.

#### CertificateGeneration

Using `generate.sh` from folder `CertificateGeneration` in this repository.

```
cd CertificateGeneration
wget https://raw.githubusercontent.com/eclipse-arrowhead/core-java-spring/master/certificates/master.crt
wget https://raw.githubusercontent.com/eclipse-arrowhead/core-java-spring/master/certificates/master.p12

## Create variables.sh
cat <<EOF > variables.sh
#!/bin/bash

export MASTER_PASSWORD="123456"
export PASSWORD="PASSWORD"
export DOMAIN="ctu"
export CLOUD="prem"
export FOLDER="./certificates/"
EOF

## PREMCompiler
## Update SAN to match the computer (append name and IP)
SAN="dns:localhost,ip:127.0.0.1" bash generate.sh -a PREMCompiler

mkdir -p ../CompilerClient/certificates
mv ./certificates/PREMCompiler* ../CompilerClient/certificates/

## PREMScheduler
## Update SAN to match the computer (append name and IP)
SAN="dns:localhost,ip:127.0.0.1" bash generate.sh -a PREMScheduler

mkdir -p ../SchedulerServer/certificates
mv ./certificates/PREMScheduler* ../SchedulerServer/certificates/

cd ..
```

### Adjust configuration scripts

Each system configuration script in folder `docker/core_system_config` of the `core-java-spring` repo has to be modified. (This does not hold for pure Java build, as in that case it has be probably specified prior to the build?).

In here, you want to modify:

* `server.address` to match IP of your server,
* `domain.name` to match your IP,
* `server.ssl.key-store` to match the certificate location,
* `server.ssl.key-store-password` to match your secret certificate key store password,
* `server.ssl.key-alias` to match service name (use full name, e.g., `service_registry.demo.ltu.arrowhead.eu`, because of [#250](https://github.com/eclipse-arrowhead/core-java-spring/issues/250),
* `server.ssl.key-password` to match certificate password,
* `server.ssl.trust-store` to match trust store certificate location, and
* `server.ssl.trust-store-password` to match trust store password.

### Adjust docker-compose configuration file (Docker only)

In here, update `MYSQL_ROOT_PASSWORD` and add correct mapping of certificate files, e.g.:

```
volumes:
  - ./<CERTIFICATE FOLDER>/authorization.p12:/authorization/authorization.p12
  - ./<CERTIFICATE FOLDER>/truststore.p12:/authorization/truststore.p12
```

Note, that Service Registry is in folder `serviceregistry`, but the certificate is named `service_registry`.
