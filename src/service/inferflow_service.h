#pragma once
#include "sslib/http_server.h"
#include "sslib/thread.h"
#include "transformer/inference_engine.h"
#include "transformer/service_data.h"
#include "tools/profiler.h"
#include "json.hpp"
#include "readerwriterqueue/readerwriterqueue.h"

INFER_FLOW_BEGIN
TRANSFORMER_BEGIN

using namespace std;
using namespace sslib;

class BaseHandler;

class InferFlowServiceCore : public sslib::Thread
{
public:
    struct Config
    {
        int http_port = 8080;
        int worker_count = 16;
        bool is_study_mode = false;

        transformer::InferenceConfig engine_config;
    };

    enum class FunctionId
    {
        ProcessQuery = 0,
        GetStat
    };

    struct QueryResult
    {
        string text;
        bool is_end = false;
    };

public:
    string Version() const;

    const Config& config() const {
        return config_;
    }

    std::mutex json_parsr_lock_;
    JsonParser json_parser_;
protected:
    Config config_;

    transformer::InferenceEngine engine_;
    WStrMap<FunctionId> fn_map_;

    map<int, moodycamel::BlockingReaderWriterQueue<QueryResult>*> query_to_result_;
    std::mutex result_lock_;

public:
    bool Init(const string &config_path);

    Socket::RetCode HandleRequest(BaseHttpServer::HttpResponseWriter &writer,
        const InferFlowRequest &request, bool is_openai_mode);
    bool HandleRequest(string &response, const InferFlowRequest &request, FunctionId fn, bool is_openai_mode);

    bool ParseRequest(InferFlowRequest &request, const wstring &request_str);

    FunctionId GetFunctionId(const string &url, const InferFlowRequest &request) const;

    void LogHttpHeader(const HttpRequest &request) const;

    bool HandleRequest_ProcessQueryCabby(string &response, const InferFlowRequest &request, bool is_openai_mode);

    Socket::RetCode HandleRequest_Inner(BaseHttpServer::HttpResponseWriter *writer,
        InferFlowResponseChunk *chunk_ptr, const InferFlowRequest &request, bool is_openai_mode);

protected:
    virtual void Run() override;

    bool Infer(int max_output_len);

    bool LoadConfig(const string &config_path);

    bool HandleRequest_ProcessQuery(string &response, const InferFlowRequest &request, bool is_openai_mode);
    bool HandleRequest_GetStat(string &response, const InferFlowRequest &request);



    static int GetUtf8EndPos(const string &text);
};

class InferFlowService : public BaseHttpServer
{
public:
    static const int MAX_REQUEST_LEN = 1000 * 1000;
    
    std::vector<std::shared_ptr<BaseHandler>> handlers;
public:
    InferFlowService() {
#ifdef USE_PROFILER
        Instrumentor::Get().BeginSession("main", "profile.json");
#endif
    };
    virtual ~InferFlowService() {
#ifdef USE_PROFILER
        Instrumentor::Get().EndSession();
#endif
    };

    bool Init(const string &config_path);
    bool Start();

    string Version() const;

    virtual bool HandleRequest(HttpResponseWriter &writer,
        const HttpTask &task) override;

    InferFlowServiceCore core_;
// protected:
};

class BaseHandler
{
protected:
    inferflow::transformer::InferFlowService& inferService;
public:
    BaseHandler(inferflow::transformer::InferFlowService& service);
    virtual bool handle(sslib::BaseHttpServer::HttpResponseWriter &writer,const sslib::BaseHttpServer::HttpTask &task) = 0;
    virtual ~BaseHandler();
};


class OpenAI_Completion: public BaseHandler
{
protected:
    const std::string url = "/chat/completions";
public:
    OpenAI_Completion(inferflow::transformer::InferFlowService& service): BaseHandler(service) {};
    bool handle(sslib::BaseHttpServer::HttpResponseWriter &writer, const sslib::BaseHttpServer::HttpTask &task) override;
};

class Cabby_Completion: public BaseHandler
{
protected:
    const std::string url = "/v1/completions";
public:
    Cabby_Completion(inferflow::transformer::InferFlowService& service): BaseHandler(service) {};
    bool handle(sslib::BaseHttpServer::HttpResponseWriter &writer, const sslib::BaseHttpServer::HttpTask &task) override;
    bool parseRequestTabby(InferFlowRequest &request,const wstring &request_str);
    bool handleRequest_ProcessQueryCabby(string &response, const InferFlowRequest &request, bool is_openai_mode);
};

class Cabby_CheckHealth: public BaseHandler
{
protected:
    const std::string url = "/v1/health";
    struct HealthState
    {
        std::wstring model;
        std::wstring chat_model;
        std::wstring chat_device;
        std::wstring device;
        std::wstring arch;
        std::wstring cpu_info;
        int cpu_count;
        std::vector<std::wstring> cuda_devices;
        std::wstring version;
        bool webserver;
    };
public:
    Cabby_CheckHealth(inferflow::transformer::InferFlowService& service): BaseHandler(service) {};
    bool handle(sslib::BaseHttpServer::HttpResponseWriter &writer, const sslib::BaseHttpServer::HttpTask &task) override;
    std::wstring response_health(HealthState& healthState);
};

class Ollama_Tags: public BaseHandler
{
protected:
    const std::string url = "/api/tags";
    struct ServiceState
    {
        std::string name;
        std::string modified_at;
        long int size;
        std::string digest;
        std::string details_format;
        std::string details_family;
        std::string details_families;
        std::string details_parameter_size;
        std::string details_quantization_level;
    };
public:
    Ollama_Tags(inferflow::transformer::InferFlowService& service): BaseHandler(service) {};
    bool handle(sslib::BaseHttpServer::HttpResponseWriter &writer, const sslib::BaseHttpServer::HttpTask &task) override;
    std::wstring responseServiceState(ServiceState& healthState);
};

class Ollama_Generate: public BaseHandler
{
protected:
    const std::string url = "/api/generate";
    struct ServiceState
    {
        std::string name;
        std::string modified_at;
        long int size;
        std::string digest;
        std::string details_format;
        std::string details_family;
        std::string details_families;
        std::string details_parameter_size;
        std::string details_quantization_level;
    };
public:
    Ollama_Generate(inferflow::transformer::InferFlowService& service): BaseHandler(service) {};
    bool handle(sslib::BaseHttpServer::HttpResponseWriter &writer, const sslib::BaseHttpServer::HttpTask &task) override;
    bool parseRequest(InferFlowRequest &request,const wstring &request_str);
    bool handleRequest_ProcessQuery(string &response, const InferFlowRequest &request, bool is_openai_mode);
};

TRANSFORMER_END
INFER_FLOW_END

