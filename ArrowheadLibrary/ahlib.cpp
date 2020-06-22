#include "ahlib.h"

#include <iostream>
#include <curl/curl.h>
#include <json-c/json.h>
#include <fstream>
#include <sstream>

/* Need to be out of class, just calls the right member function */
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata){
    return ((ArrowheadBase *) userdata)->httpsResponseCallback(ptr, size * nmemb);
}

ArrowheadBase::ArrowheadBase(std::string certConfigFile) {
    std::ifstream f(certConfigFile);
    if(!f.good()) std::cout << "Cannot open " << certConfigFile << std::endl;
    std::stringstream buffer;
    buffer << f.rdbuf();
    struct json_object *jsonString = json_tokener_parse(buffer.str().c_str());

    if(jsonString == NULL){
        std::cout << "Error: Cannot parse json: " << buffer.str().c_str() << std::endl;
        return;
    }

    /* Certificate authority */
    struct json_object *json_cacert;
    if(!json_object_object_get_ex(jsonString, "cacert", &json_cacert)){
        std::cout << "Error: Could not find \"cacert\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }

    cacertPath = std::string(json_object_get_string(json_cacert));

    /* Client certificate */
    struct json_object *json_clcert;
    if(!json_object_object_get_ex(jsonString, "clcert", &json_clcert)){
        std::cout << "Error: Could not find \"clcert\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }

    clcertPath = std::string(json_object_get_string(json_clcert));

    /* Private key */
    struct json_object *json_privkey;
    if(!json_object_object_get_ex(jsonString, "privkey", &json_privkey)){
        std::cout << "Error: Could not find \"privkey\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }

    privkeyPath = std::string(json_object_get_string(json_privkey));

    /* Private key password */
    struct json_object *json_privpass;
    if(!json_object_object_get_ex(jsonString, "privpass", &json_privpass)){
        std::cout << "Error: Could not find \"privpass\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }

    privkeyPass = std::string(json_object_get_string(json_privpass));
}

int ArrowheadBase::httpsRequest(std::string pdata, std::string paddr, std::string pmethod) {
    printf("Sending HTTPS request: %s\n", paddr.c_str());

    int http_code = 0;
    CURLcode res;
    CURL *curl;
    struct curl_slist *headers = NULL;
    std::string agent;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();

    if(curl){
        agent = "libcurl/"+std::string(curl_version_info(CURLVERSION_NOW)->version);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, agent.c_str());

        headers = curl_slist_append(headers, "Expect:");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        if(pmethod == "POST")
            curl_easy_setopt(curl, CURLOPT_POST, true);
        else
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, pmethod.c_str());


        //---------------HTTPS SECTION--------------------------------------------------------
        //--verbose
        if ( curl_easy_setopt(curl, CURLOPT_VERBOSE,        1L)            != CURLE_OK)
            printf("error: CURLOPT_VERBOSE\n");
        //--insecure
        if ( curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L)            != CURLE_OK)
            printf("error: CURLOPT_SSL_VERIFYPEER\n");
        if ( curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L)            != CURLE_OK)
            printf("error: CURLOPT_SSL_VERIFYHOST\n");
        //--cert
        if ( curl_easy_setopt(curl, CURLOPT_SSLCERT,        clcertPath.c_str())  != CURLE_OK)
            printf("error: CURLOPT_SSLCERT\n");
        //--cert-type
        if ( curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE,    "PEM")         != CURLE_OK)
            printf("error: CURLOPT_SSLCERTTYPE\n");
        //--key
        //if ( curl_easy_setopt(curl, CURLOPT_SSLKEY,         "keys/tempsensor.testcloud1.private.key") != CURLE_OK)
        if ( curl_easy_setopt(curl, CURLOPT_SSLKEY,         privkeyPath.c_str()) != CURLE_OK)
            printf("error: CURLOPT_SSLKEY\n");
        //--key-type
        if ( curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE,     "PEM")         != CURLE_OK)
            printf("error: CURLOPT_SSLKEYTYPE\n");
        //--pass
        if ( curl_easy_setopt(curl, CURLOPT_KEYPASSWD,      privkeyPass.c_str())       != CURLE_OK)
            printf("error: CURLOPT_KEYPASSWD\n");
        //--cacert
        if ( curl_easy_setopt(curl, CURLOPT_CAINFO,         cacertPath.c_str())  != CURLE_OK)
            printf("error: CURLOPT_CAINFO\n");
        //
        //---------------END OF HTTPS SECTION-------------------------------------------------

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pdata.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

        curl_easy_setopt(curl, CURLOPT_URL, paddr.c_str());

        /* Execute in blocking manner */
        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            std::cout <<"Error: curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return http_code;
}



ArrowheadProvider::ArrowheadProvider(std::string certConfigFile, std::string providerConfigFile) : ArrowheadBase(certConfigFile), providerRegistered(false) {
    std::ifstream f(providerConfigFile);
    if(!f.good()) std::cout << "Cannot open " << providerConfigFile << std::endl;
    std::stringstream buffer;
    buffer << f.rdbuf();

    struct json_object *jsonString = json_tokener_parse(buffer.str().c_str());
    if(jsonString == NULL){
        std::cout << "Error: Cannot parse json: " << buffer.str().c_str() << std::endl;
        return;
    }

    struct json_object *json_addrv4;
    if(!json_object_object_get_ex(jsonString, "serverAddressIPV4", &json_addrv4)){
        std::cout << "Error: Could not find \"serverAddressIPV4\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }
    config.serverAddressIPV4 = std::string(json_object_get_string(json_addrv4));

    struct json_object *json_addrv6;
    if(!json_object_object_get_ex(jsonString, "serverAddressIPV6", &json_addrv6)){
        std::cout << "Error: Could not find \"serverAddressIPV6\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }
    config.serverAddressIPV6 = std::string(json_object_get_string(json_addrv6));

    struct json_object *json_port;
    if(!json_object_object_get_ex(jsonString, "serverPort", &json_port)){
        std::cout << "Error: Could not find \"serverPort\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }
    config.serverPort = json_object_get_int(json_port);

    struct json_object *json_customURL;
    if(!json_object_object_get_ex(jsonString, "customURL", &json_customURL)){
        std::cout << "Error: Could not find \"customURL\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }
    config.customURL = std::string(json_object_get_string(json_customURL));

    struct json_object *json_systemName;
    if(!json_object_object_get_ex(jsonString, "systemName", &json_systemName)){
        std::cout << "Error: Could not find \"systemName\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }
    config.systemName = std::string(json_object_get_string(json_systemName));

    struct json_object *json_serviceDefinition;
    if(!json_object_object_get_ex(jsonString, "serviceDefinition", &json_serviceDefinition)){
        std::cout << "Error: Could not find \"serviceDefinition\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }
    config.serviceDefinition = std::string(json_object_get_string(json_serviceDefinition));

    struct json_object *json_serviceInterface;
    if(!json_object_object_get_ex(jsonString, "serviceInterface", &json_serviceInterface)){
        std::cout << "Error: Could not find \"serviceInterface\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }
    config.serviceInterface = std::string(json_object_get_string(json_serviceInterface));

    struct json_object *json_cloudBaseURL;
    if(!json_object_object_get_ex(jsonString, "serviceRegistryBaseURL", &json_cloudBaseURL)){
        std::cout << "Error: Could not find \"serviceRegistryBaseURL\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }
    serviceRegistryBaseURL = std::string(json_object_get_string(json_cloudBaseURL));

    createJSONServiceDefinition();
}

int ArrowheadProvider::registerToServiceRegistry() {
    return httpsRequest(jsonServiceDefinition , serviceRegistryBaseURL + "register", "POST");
}

int ArrowheadProvider::unRegisterFromServiceRegistry() {
    std::string params = "service_definition=" + config.serviceDefinition +
                          "&system_name=" + config.systemName +
                          "&address=" + (config.serverAddressIPV4.size() ? config.serverAddressIPV4 : config.serverAddressIPV6) +
                          "&port=" + std::to_string(config.serverPort);
    return httpsRequest("", serviceRegistryBaseURL + "unregister?" + params, "DELETE");
}

void ArrowheadProvider::createJSONServiceDefinition()
{
//Expected content, example:
/*
{
 "serviceDefinition": "IndoorTemperature",
 "serviceUri": "temperature",
 "endOfValidity": "2019-12-05T12:00:00",
 "secure": "TOKEN",
 "version": 1,

 "providerSystem":
 {
   "systemName": "InsecureTemperatureSensor",
   "address": "192.168.0.2",
   "port": 8080,
   "authenticationInfo": "eyJhbGciOiJIUzI1Ni..."
 },

 "metadata": {
   "unit": "celsius"
},

 "interfaces": [
   "HTTP-SECURE-JSON"
 ]
}
*/
    json_object *jobj            = json_object_new_object();
    json_object *providerSystem  = json_object_new_object();
    json_object *jstring;
    json_object *jint;

    jstring = json_object_new_string(config.serviceDefinition.c_str());
    json_object_object_add(jobj, "serviceDefinition", jstring);

    jstring = json_object_new_string(config.customURL.c_str());
    json_object_object_add(jobj, "serviceUri", jstring);

    jint = json_object_new_int(1);
    json_object_object_add(jobj, "version", jint);

/*
*   providerSystem section
*/
    jstring = json_object_new_string(config.systemName.c_str());
    json_object_object_add(providerSystem, "systemName", jstring);

    jstring = json_object_new_string( config.serverAddressIPV4.size() != 0 ? config.serverAddressIPV4.c_str() : config.serverAddressIPV6.c_str());
    json_object_object_add(providerSystem, "address", jstring);

    jstring = json_object_new_string("NOT_SECURE");
    json_object_object_add(jobj, "secure", jstring);

    jint = json_object_new_int(config.serverPort);
    json_object_object_add(providerSystem, "port", jint);

    json_object_object_add(jobj, "providerSystem", providerSystem);

/*
*   Interfaces, Metadata
*/

    json_object *jarray = json_object_new_array();

    jstring = json_object_new_string("HTTP-INSECURE-JSON");

    json_object_array_add(jarray, jstring);
    json_object_object_add(jobj, "interfaces", jarray);

/*
*   Return
*/

    std::cout << json_object_to_json_string(jobj) << std::endl;

    jsonServiceDefinition = json_object_to_json_string(jobj);

}

bool ArrowheadProvider::registerProvider() {
    std::cout << "Registering provider" << std::endl;
    int returnValue = registerToServiceRegistry();
    if (returnValue == 201 /*Created*/){
        providerRegistered = true;
        return true;
    }
    else{
        std::cout << "Registering return code: " << returnValue << std::endl;
        std::cout << "Already registered?" << std::endl;
        std::cout << "Try re-registration" << std::endl;;

        returnValue = unRegisterFromServiceRegistry();

        if (returnValue == 200 /*OK*/ || returnValue == 204 /*No Content*/) {
            std::cout << "Unregistration is successful" << std::endl;
        }
        else {
            std::cout << "Unregistration is unsuccessful - code: " << returnValue << std::endl;
            return false;
        }

        returnValue = registerToServiceRegistry();
        if (returnValue == 201 /*Created*/){
            providerRegistered = true;
            return true;
        }else {
            return false; //unsuccessful registration
        }
    }
}

size_t ArrowheadProvider::httpsResponseCallback(char *ptr, size_t size) {
    std::cout << std::string(ptr) << std::endl;
    struct json_object *obj = json_tokener_parse(ptr);
    std::cout << std::string(ptr) << std::endl;
    if(obj == NULL){
        std::cout << "Error: could not parse serviceregistry response" << std::endl;
        return 1;
    }

    struct json_object *jProvider;
    if(!json_object_object_get_ex(obj, "provider", &jProvider)){
        std::cout << "Error: could not parse provider section" << std::endl;
        return 1;
    }

    struct json_object *jService;
    if(!json_object_object_get_ex(obj, "serviceDefinition", &jService)){
        std::cout << "Error: could not parse serviceDefinition section" << std::endl;
        return 1;
    }

    struct json_object *jInterfaceArray;
    if(!json_object_object_get_ex(obj, "interfaces", &jInterfaceArray)){
        std::cout << "Error: could not parse interfaces" << std::endl;
        return 1;
    }
    struct json_object *jInterface = json_object_array_get_idx(jInterfaceArray, 0);

    struct json_object *jId;
    struct json_object *jServiceId;
    struct json_object *jInterfaceId;

    if(!json_object_object_get_ex(jProvider,  "id",         &jId))         {std::cout << "Error: could not find id" << std::endl;           return 1;}
    if(!json_object_object_get_ex(jService,   "id",         &jServiceId))  {std::cout << "Error: could not find serviceId" << std::endl;    return 1;}
    if(!json_object_object_get_ex(jInterface, "id",         &jInterfaceId)){std::cout << "Error: could not find interface id" << std::endl; return 1;}

    config.id = json_object_get_int(jId);
    config.serviceDefinitionId = json_object_get_int(jServiceId);
    config.interfaceId = json_object_get_int(jInterfaceId);
    return size;
}

void ArrowheadProvider::printProviderIds() {
    std::cout << "\"providerInterfaceId\" : " << config.interfaceId << "," << std::endl
              << "\"providerId\" : " << config.id << "," << std::endl
              << "\"providerServiceDefinitionId\" : " << config.serviceDefinitionId << std::endl;
}


ArrowheadConsumer::ArrowheadConsumer(std::string certConfigFile, std::string consumerConfigFile) : ArrowheadBase(certConfigFile) {
    std::ifstream f(consumerConfigFile);
    if(!f.good()) std::cout << "Cannot open " << consumerConfigFile << std::endl;
    std::stringstream buffer;
    buffer << f.rdbuf();

    struct json_object *jsonString = json_tokener_parse(buffer.str().c_str());

    if(jsonString == NULL){
        std::cout << "Error: Cannot parse json: " << buffer.str().c_str() << std::endl;
        return;
    }

    struct json_object *json_orchestratorURL;
    if(!json_object_object_get_ex(jsonString, "orchestratorURL", &json_orchestratorURL)){
        std::cout << "Error: Could not find \"orchestratorURL\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }

    orchestratorURL = std::string(json_object_get_string(json_orchestratorURL));

    struct json_object *json_consID;
    if(!json_object_object_get_ex(jsonString, "consumerID", &json_consID)){
        std::cout << "Error: Could not find \"consumerID\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }

    consumerId = std::string(json_object_get_string(json_consID));

    struct json_object *json_reqForm;
    if(!json_object_object_get_ex(jsonString, "requestForm", &json_reqForm)){
        std::cout << "Error: Could not find \"requestForm\" in " << std::endl << buffer.str().c_str() << std::endl;
        return;
    }

    consumerRequest = std::string(json_object_get_string(json_reqForm));
}

ProviderInfo ArrowheadConsumer::findProvider() {
    int returnCode = httpsRequest(consumerRequest, orchestratorURL, "POST");
    std::cout << "Return code: " << returnCode << std::endl;
    return info;
}

size_t ArrowheadConsumer::httpsResponseCallback(char *ptr, size_t size) {
    struct json_object *obj = json_tokener_parse(ptr);
    std::cout << std::string(ptr) << std::endl;
    if(obj == NULL){
        std::cout << "Error: could not parse orchestration response" << std::endl;
        return 1;
    }

    struct json_object *jResponseArray;
    if(!json_object_object_get_ex(obj, "response", &jResponseArray)){
        std::cout << "Error: could not parse response" << std::endl;
        return 1;
    }

    struct json_object *jResponse = json_object_array_get_idx(jResponseArray, 0);
    struct json_object *jProvider;

    if(!json_object_object_get_ex(jResponse, "provider", &jProvider)){
        std::cout << "Error: could not parse provider section" << std::endl;
        return 1;
    }

    struct json_object *jId;
    struct json_object *jAddr;
    struct json_object *jPort;
    struct json_object *jService;
    struct json_object *jServiceId;
    struct json_object *jUri;

    if(!json_object_object_get_ex(jProvider, "id",         &jId))        {std::cout << "Error: could not find id" << std::endl;         return 1;}
    if(!json_object_object_get_ex(jProvider, "address",    &jAddr))      {std::cout << "Error: could not find address" << std::endl;    return 1;}
    if(!json_object_object_get_ex(jProvider, "port",       &jPort))      {std::cout << "Error: could not find port" << std::endl;       return 1;}
    if(!json_object_object_get_ex(jResponse, "service",    &jService))   {std::cout << "Error: could not find service" << std::endl;    return 1;}
    if(!json_object_object_get_ex(jService,  "id",         &jServiceId)) {std::cout << "Error: could not find serviceId" << std::endl;  return 1;}
    if(!json_object_object_get_ex(jResponse, "serviceUri", &jUri))       {std::cout << "Error: could not find serviceURI" << std::endl; return 1;}

    struct json_object *jInterfaceArray;
    if(!json_object_object_get_ex(jResponse, "interfaces", &jInterfaceArray)){
        std::cout << "Error: could not parse interfaces" << std::endl;
        return 1;
    }
    struct json_object *jInterface = json_object_array_get_idx(jInterfaceArray, 0);
    struct json_object *jInterfaceId;
    if(!json_object_object_get_ex(jInterface, "id", &jInterfaceId)){
        std::cout << "Error: could not find interface id" << std::endl;
        return 1;
    }

    info.id = json_object_get_int(jId);
    info.serverAddressIPV4 = std::string(json_object_get_string(jAddr));
    info.serverAddressIPV6 = std::string(json_object_get_string(jAddr));
    info.serverPort    = json_object_get_int(jPort);
    info.serviceDefinitionId = json_object_get_int(jServiceId);
    info.customURL     = std::string(json_object_get_string(jUri));
    info.interfaceId = json_object_get_int(jInterfaceId);
    return size;
}