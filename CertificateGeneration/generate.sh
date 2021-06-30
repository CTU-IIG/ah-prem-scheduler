#!/bin/bash


cd "$(dirname "$0")" || exit
source "variables.sh"
source "lib_certs.sh"


## Variables
# DOMAIN
# FOLDER = "./certificates/"
# CLOUD

if ! test -d ${FOLDER}; then
    mkdir -r "$FOLDER"
fi

if test $# -gt 0; then
    SYSTEM=$1;
else
    echo "Expected system name within the arguments." >&2
    exit 2;
fi

# Generating certificates for SYSTEM.CLOUD.DOMAIN.arrowhead.eu



## 1) Generate root certificate keystore
echo -n "Step 1: Root certificate "

if test -f "root.p12"; then
    echo "FOUND";
else
#    create_root_keystore \
#        "${FOLDER}root.p12" "arrowhead.eu"
#    echo "GENERATED";
    echo "Download 'root.p12' and 'root.crt' (also called 'master') from arrowhead-f repository." >&2
    exit 1;
fi


## 2) Generate truststore
echo -n "Step 2: Truststore "

if test -f "${FOLDER}truststore.p12"; then
    echo "FOUND";
else
    create_truststore \
        "${FOLDER}truststore.p12" "root.crt" "arrowhead.eu"
    echo "GENERATED";
fi


## 3) Generate cloud keystore
echo -n "Step 3: Cloud keystore "

if test -f "${FOLDER}${CLOUD}.p12"; then
    echo "FOUND";
else
    create_cloud_keystore \
        "root.p12" "arrowhead.eu" \
        "${FOLDER}${CLOUD}.p12" "${CLOUD}.${DOMAIN}.arrowhead.eu"
    echo "GENERATED";
fi


## 4) Generate system certificate
echo -n "Step 4: System certificates "

while test $# -gt 0; do
    SYSTEM=$1

    if test -f "${FOLDER}${SYSTEM}.p12"; then
        echo "${SYSTEM} : FOUND";
    else
        create_system_keystore \
            "root.p12" "arrowhead.eu" \
            "${FOLDER}${CLOUD}.p12" "${CLOUD}.${DOMAIN}.arrowhead.eu" \
            "${FOLDER}${SYSTEM}.p12" "${SYSTEM}.${CLOUD}.${DOMAIN}.arrowhead.eu" \
            "dns:localhost,ip:127.0.0.1"
        echo "${SYSTEM} : GENERATED";
    fi
    shift
done

## 5) Generate sysop certificate
echo -n "Step 5: Sysop certificate "

if test -f "${FOLDER}sysop.p12"; then
    echo "FOUND";
else
    create_sysop_keystore \
        "root.p12" "arrowhead.eu" \
        "${FOLDER}${CLOUD}.p12" "${CLOUD}.${DOMAIN}.arrowhead.eu" \
        "${FOLDER}sysop.p12" "sysop.${CLOUD}.${DOMAIN}.arrowhead.eu"
    echo "GENERATED";
fi

