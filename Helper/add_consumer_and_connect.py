#!/usr/bin/python3

import pycurl
from io import BytesIO
import json


serviceRegistryMgmtURL = "https://10.35.127.242:8443/serviceregistry/mgmt/systems"
authorizationMgmtURL = "https://10.35.127.242:8445/authorization/mgmt/intracloud"
consumerId = -1

def sendHttpsPOSTRequest(address, data):
    with open('certificates.json') as json_file:
        certificates_data = json.load(json_file)

    b_obj = BytesIO()
    crl = pycurl.Curl()

    # Set parameters
    crl.setopt(pycurl.VERBOSE, 1)
    crl.setopt(pycurl.HTTPHEADER, ["Expect:", "Content-Type: application/json"])
    crl.setopt(pycurl.POST, True)
    crl.setopt(pycurl.SSL_VERIFYPEER, 0)
    crl.setopt(pycurl.SSL_VERIFYHOST, 0)
    crl.setopt(pycurl.SSLKEYTYPE, "PEM")
    crl.setopt(pycurl.SSLCERT, certificates_data['clcert'])
    crl.setopt(pycurl.SSLKEY, certificates_data['privkey'])
    crl.setopt(pycurl.KEYPASSWD, certificates_data['privpass'])
    crl.setopt(pycurl.CAINFO, certificates_data['cacert'])

    crl.setopt(pycurl.POSTFIELDS, data)
    crl.setopt(pycurl.POSTFIELDSIZE, -1)

    crl.setopt(pycurl.URL, address)

    # Write bytes that are utf-8 encoded
    crl.setopt(crl.WRITEDATA, b_obj)

    # Perform a file transfer
    crl.perform()

    # End curl session
    crl.close()

    # Get the content stored in the BytesIO object (in byte characters)
    get_body = b_obj.getvalue()

    # Decode the bytes stored in get_body to HTML and print the result
    print('Output of the request:\n%s' % get_body.decode('utf8'))
    return get_body.decode('utf8')

with open('config.json') as json_file:
    config_data = json.load(json_file)

if consumerId < 0:
    consumerSystemData = {}
    consumerSystemData['address'] = config_data['consumerAddress']
    consumerSystemData['authenticationInfo'] = config_data['consumerPublicCertificate']
    consumerSystemData['port'] = config_data['consumerPort']
    consumerSystemData['systemName'] = config_data['consumerName']

    returnValue = sendHttpsPOSTRequest(serviceRegistryMgmtURL, json.dumps(consumerSystemData))

    returnJson = json.loads(returnValue)

    consumerId = returnJson['id']

    if 'errorCode' in returnJson:
        print("You have to check swagerrui, find system id and delete manually or fill the number at the beginning of the file")


authorizationData = {}
authorizationData['consumerId'] = consumerId
authorizationData['interfaceIds'] = []
authorizationData['interfaceIds'].append(config_data['providerInterfaceId'])
authorizationData['providerIds'] = []
authorizationData['providerIds'].append(config_data['providerId'])
authorizationData['serviceDefinitionIds'] = []
authorizationData['serviceDefinitionIds'].append(config_data['providerServiceDefinitionId'])

sendHttpsPOSTRequest(authorizationMgmtURL, json.dumps(authorizationData))