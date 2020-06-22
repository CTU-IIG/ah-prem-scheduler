#ifndef ARROWHEADLIBRARY_AHLIB_H
#define ARROWHEADLIBRARY_AHLIB_H

#include <string>

struct ProviderInfo{
    int id;
    int interfaceId;
    int serviceDefinitionId;
    std::string serverAddressIPV4;
    std::string serverAddressIPV6;
    unsigned short serverPort;
    std::string customURL;
    std::string systemName;
    std::string serviceDefinition;
    std::string serviceInterface;
    int serviceInterfaceId;
};

class ArrowheadBase{
public:
    ArrowheadBase(std::string certConfigFile);;
    int httpsRequest(std::string pdata, std::string paddr, std::string pmethod);
    virtual size_t httpsResponseCallback(char *ptr, size_t size) = 0;
private:
    std::string clcertPath;
    std::string privkeyPath;
    std::string privkeyPass;
    std::string cacertPath;
};

class ArrowheadProvider : public ArrowheadBase{
public:
    ArrowheadProvider(std::string certConfigFile, std::string providerConfigFile);
    bool registerProvider();
    size_t httpsResponseCallback(char *ptr, size_t size) override;
    void printProviderIds();
private:
    ProviderInfo config;

    std::string serviceRegistryBaseURL;

    std::string jsonServiceDefinition;

    bool providerRegistered;

    int registerToServiceRegistry();
    int unRegisterFromServiceRegistry();

    void createJSONServiceDefinition();
};

class ArrowheadConsumer : public ArrowheadBase{
public:
    ArrowheadConsumer(std::string certConfigFile, std::string consumerConfigFile);
    ProviderInfo findProvider();
    size_t httpsResponseCallback(char *ptr, size_t size) override;
private:
    std::string consumerId;
    std::string consumerRequest;

    std::string orchestratorURL;

    ProviderInfo info;
};

#endif //ARROWHEADLIBRARY_AHLIB_H
