# Certificate generation scripts

Developed by: [https://github.com/emanuelpalm](https://github.com/emanuelpalm)

Root certificates are downloaded from: [https://github.com/arrowhead-f/core-java-spring/certificates](https://github.com/arrowhead-f/core-java-spring/certificates)


## Usage

### Variable definition

At first, create a bash file containing variables used for certificate generation. They are in a separate file in order to have some boundary between public and private files.

Example:

```bash
#!/bin/bash

export PASSWORD="PASSWORD USED FOR THE CERTIFICATES"
export DOMAIN="ltu"
export CLOUD="relay"
export FOLDER="./certificates/"
```

### System certificates generation

Certificates for the main components of Arrowhead can be created by executing:

```
bash generate.sh "authorization" "contract_proxy" "data_consumer" "event_handler" "gatekeeper" "gateway" "orchestrator" "service_registry"
```

### Custom certificate generation

Any other certificates (for your own producers and consumers) are created by:

```
bash generate.sh [SERVICE NAME]
```