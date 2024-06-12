#include "inferflow_service.h"
#include <sstream>
#include <iomanip>

INFER_FLOW_BEGIN
TRANSFORMER_BEGIN

BaseHandler::BaseHandler(inferflow::transformer::InferFlowService& service)
        : inferService(service)
{}

BaseHandler::~BaseHandler() {}


bool OpenAI_Completion::handle(sslib::BaseHttpServer::HttpResponseWriter &writer, const sslib::BaseHttpServer::HttpTask &task) 
{
    if (task.request.url.find(this->url) == std::string::npos)
    {
        return false;
    }
    else
    {
        bool is_unknown_type = task.request.type == sslib::HttpRequest::Method::Unknown;
        sslib::HttpResponse response;
        bool ret = true;
        response.status_code = 200;
        bool is_valid_query = false;
        const auto &http_request = task.request;
        std::string url = task.request.url;
        // OpenAI Format
        is_valid_query = true;
        uint32_t request_hash = 0;
        inferflow::transformer::InferFlowRequest request;
        // InferFlowResponseChunk res_chunk;
        std::wstring error_msg;
        std::string content_type = task.request.type == sslib::HttpRequest::Method::Get
                ? "text/html" : "application/json";
        
        if ((int)task.request.body.size() > inferflow::transformer::InferFlowService::MAX_REQUEST_LEN)
        {
            error_msg = L"error.too_long_request";
        }
        request_hash = sslib::Hash::HashCodeUInt32(task.request.body);
        std::wstring request_body = sslib::StringUtil::Utf8ToWideStr(task.request.body);
        //Parse the request (assuming JSON format)
        ret = inferService.core_.ParseRequest(request, request_body);
        if (!ret)
        {
            error_msg = L"error.invalid_request_format";
            is_valid_query = false;
            LogStatusInfo(L"error.invalid_request_format:\t%u\t%ls", request_hash, task.request.body.c_str());
        }
        else
        {   
            std::wstring encoded_request = sslib::JsonBuilder::EncodeString(request_body);
            LogStatusInfo(L"query_start\t%u\t%ls", request_hash, encoded_request.c_str());
            inferflow::transformer::InferFlowServiceCore::FunctionId fn = inferService.core_.GetFunctionId(http_request.url, request);

            bool is_std_fn = ((int)fn == 0);
            bool be_keep_alive = !is_unknown_type && !task.hdr.is_close;

            sslib::Socket::RetCode ret_code = sslib::Socket::RetCode::Success;
            if (request.is_streaming_mode && is_std_fn)
            {
                // stream 推理
                int body_len = -1;
                ret_code = writer.WriteHeader(response.header_lines, content_type,
                    response.status_code, be_keep_alive, body_len);
                if (is_valid_query && ret_code == sslib::Socket::RetCode::Success)
                {
                    ret_code = inferService.core_.HandleRequest(writer, request, true);
                }
            }
            else
            {
                std::string response_body;
                if (is_valid_query) {
                    // 执行推理
                    ret = inferService.core_.HandleRequest(response_body, request, fn, true);
                }

                int body_len = (int)response_body.size();
                ret_code = writer.WriteHeader(response.header_lines, content_type,
                    response.status_code, be_keep_alive, body_len);
                if (is_valid_query && ret_code == sslib::Socket::RetCode::Success
                    && !response_body.empty())
                {
                    ret_code = writer.WriteString(response_body);
                }
            }
            // 是否推理成功
            return ret_code == sslib::Socket::RetCode::Success;
        }
        // 错误处理
        std::wstring response_temp;
        std::string response_body;
        inferflow::transformer::InferFlowResponseChunk res_chunk;
        res_chunk.error_text = error_msg;
        res_chunk.ToJson_OpenAI(response_temp);
        int body_len = (int)response_temp.size();
        sslib::Socket::RetCode ret_code = writer.WriteHeader(response.header_lines, content_type,
                        response.status_code, true, body_len);
        sslib::StringUtil::ToUtf8(response_body, response_temp);
        if (ret_code == sslib::Socket::RetCode::Success && body_len > 0)
        {
            ret_code = writer.WriteString(response_body);
        }
        return ret_code == sslib::Socket::RetCode::Success;
    }
}

bool Cabby_CheckHealth::handle(sslib::BaseHttpServer::HttpResponseWriter &writer, const sslib::BaseHttpServer::HttpTask &task)
{
    if (task.request.url.find(this->url) == std::string::npos)
    {
        return false;
    }
    else
    {
        /*
            response: status = 200, description = "Success", body = HealthState, content_type = "application/json"
            HealthState:
                model:
                chat_model:
                chat_device
                device:
                arch
                cpu_info
                cpu_count
                cuda_devices
                version: 
                webserver: bool
            */
            LogStatusInfo(L"query health");
            sslib::HttpResponse response;

            response.status_code = 200;
            std::string content_type = task.request.type == sslib::HttpRequest::Method::Get
                    ? "text/html" : "application/json";
            inferflow::transformer::InferFlowResponseChunk res_chunk;
            HealthState healthState = {L"gpt-3.5-turbo", L"gpt-3.5-turbo", L"cuda",L"cuda", L"decoder-only", L"intel", 8, {L"cuda"}, L"1.0", false};
            std::wstring health_json = response_health(healthState);
            int body_len = health_json.size();
            sslib::Socket::RetCode ret_code = writer.WriteHeader(response.header_lines, content_type,
                            response.status_code, true, body_len);
            std::string body_utf8;
            sslib::StringUtil::ToUtf8(body_utf8, health_json);
            if (ret_code == sslib::Socket::RetCode::Success && body_len > 0)
            {
                ret_code = writer.WriteString(body_utf8);
            }
            return ret_code == sslib::Socket::RetCode::Success;
    }
}

std::wstring Cabby_CheckHealth::response_health(HealthState& healthState)
{
    std::basic_stringstream<wchar_t> ss;
    ss << L"{";
    ss << L"\"model\":";
    inferflow::transformer::JsonBuilder::AppendFieldValue(ss, healthState.model);
    ss << L",\"chat_model\":";
    inferflow::transformer::JsonBuilder::AppendFieldValue(ss, healthState.chat_model);
    ss << L",\"chat_device\":";
    inferflow::transformer::JsonBuilder::AppendFieldValue(ss, healthState.chat_device);
    ss << L",\"device\":";
    inferflow::transformer::JsonBuilder::AppendFieldValue(ss, healthState.device);
    ss << L",\"arch\":";
    inferflow::transformer::JsonBuilder::AppendFieldValue(ss, healthState.arch);
    ss << L",\"cpu_info\":";
    inferflow::transformer::JsonBuilder::AppendFieldValue(ss, healthState.cpu_info);
    ss << L",\"cpu_count\":" << healthState.cpu_count;
    ss << L",\"cuda_devices\":" << "[";
    int size = healthState.cuda_devices.size();
    for(int i = 0; i < size; i++)
    {
        inferflow::transformer::JsonBuilder::AppendFieldValue(ss, healthState.cuda_devices[i]);
        if (i < size - 1)
            ss << ",";
    }
    ss << "]";
    ss << L",\"version\":";
    inferflow::transformer::JsonBuilder::AppendFieldValue(ss, healthState.version);
    ss << L",\"webserver\":";
    ss << (healthState.webserver ? "true" : "false");
    ss << "}";
    return ss.str();
}

bool Cabby_Completion::handle(sslib::BaseHttpServer::HttpResponseWriter &writer, const sslib::BaseHttpServer::HttpTask &task)
{
    if (task.request.url.find(this->url) == std::string::npos)
    {
        return false;
    }
    else
    {
        bool is_unknown_type = task.request.type == sslib::HttpRequest::Method::Unknown;
        const auto &http_request = task.request;
        sslib::HttpResponse response;
        response.status_code = 200;
        bool is_valid_query = true;
        uint32_t request_hash = 0;
        bool ret = true;
        inferflow::transformer::InferFlowRequest request;
        // InferFlowResponseChunk res_chunk;
        std::wstring error_msg;
        std::string content_type = task.request.type == sslib::HttpRequest::Method::Get
                ? "text/html" : "application/json";
        
        if ((int)task.request.body.size() > inferflow::transformer::InferFlowService::MAX_REQUEST_LEN)
        {
            error_msg = L"error.too_long_request";
        }
        request_hash = sslib::Hash::HashCodeUInt32(task.request.body);
        std::wstring request_body = sslib::StringUtil::Utf8ToWideStr(task.request.body);
        //Parse the request (assuming JSON format)
        ret = parseRequestTabby(request, request_body);
        if (!ret)
        {
            response.status_code = 400;
            error_msg = L"error.invalid_request_format";
            is_valid_query = false;
            LogStatusInfo(L"error.invalid_request_format:\t%u\t%ls", request_hash, task.request.body.c_str());
        }
        else
        {   
            std::wstring encoded_request = sslib::JsonBuilder::EncodeString(request_body);
            LogStatusInfo(L"query_start\t%u\t%ls", request_hash, encoded_request.c_str());

            bool be_keep_alive = !is_unknown_type && !task.hdr.is_close;
            

            sslib::Socket::RetCode ret_code = sslib::Socket::RetCode::Success;
            
            std::string response_body;
            if (is_valid_query) {
                // 执行推理
                ret = handleRequest_ProcessQueryCabby(response_body, request, false);
            }

            int body_len = (int)response_body.size();
            ret_code = writer.WriteHeader(response.header_lines, content_type,
                response.status_code, be_keep_alive, body_len);
            if (is_valid_query && ret_code == sslib::Socket::RetCode::Success
                && !response_body.empty())
            {
                ret_code = writer.WriteString(response_body);
            }
            return ret_code == sslib::Socket::RetCode::Success;
        }
        // 错误处理
        std::wstring response_temp;
        std::string response_body;
        inferflow::transformer::InferFlowResponseChunk res_chunk;
        res_chunk.error_text = error_msg;
        res_chunk.ToJson_OpenAI(response_temp);
        int body_len = (int)response_temp.size();
        sslib::Socket::RetCode ret_code = writer.WriteHeader(response.header_lines, content_type,
                        response.status_code, true, body_len);
        sslib::StringUtil::ToUtf8(response_body, response_temp);
        if (ret_code == sslib::Socket::RetCode::Success && body_len > 0)
        {
            ret_code = writer.WriteString(response_body);
        }
        return ret_code == sslib::Socket::RetCode::Success;
    }
}

bool Cabby_Completion::parseRequestTabby(InferFlowRequest &request,
    const wstring &request_str)
{
    inferService.core_.json_parsr_lock_.lock(); //lock the json parser
    JsonDoc jdoc;
    bool ret = inferService.core_.json_parser_.Parse(jdoc, request_str);
    inferService.core_.json_parsr_lock_.unlock(); //unlock the json parser
    Macro_RetFalseIf(!ret);

    JsonObject jobj;
    jdoc.GetJObject(jobj);

    // parse to request instance
    request.Clear();
    //header
    JsonObject header_obj;
    jobj.GetFieldValue(header_obj, L"header", jdoc);
    ret = request.header.FromJson(header_obj, jdoc);

    wstring language, prefix, suffix, filepath, declarations;
    JsonArray relevant_snippets_from_changed_files;
    jobj.GetFieldValue(language, L"language", jdoc);

    JsonObject segments;
    jobj.GetFieldValue(segments, L"segments", jdoc);

    segments.GetFieldValue(prefix, L"prefix", jdoc);
    segments.GetFieldValue(suffix, L"suffix", jdoc);
    segments.GetFieldValue(filepath, L"filepath", jdoc);
    segments.GetFieldValue(declarations, L"declarations", jdoc);
    segments.GetFieldValue(relevant_snippets_from_changed_files, L"relevant_snippets_from_changed_files", jdoc);
    
    // wstringstream prompt_stream;
    // prompt_stream << "<fim_prefix>" << prefix 
        // << "<fim_suffix>" << suffix << "<fim_middle>"; 
    // prompt_stream << '<fix_prefix>' << prefix << '<fim_suffix>' << suffix << '<fim_middle>';
    // prompt_stream << "# language: " << language << "\n";
    // prompt_stream << prefix;

    // request.query.system_prompt;
    // request.query.response_prefix = prompt_stream.str();
    request.query.code_language = language;
    request.query.code_prefix = prefix;
    request.query.code_suffix = suffix;
    
    {
        // default
        request.max_output_len = 100;
        request.is_streaming_mode = false;
        request.temperature = 0.01;
        request.random_seed = 1234;
    }

    return true;
}

bool Cabby_Completion::handleRequest_ProcessQueryCabby(string &response,
    const InferFlowRequest &request, bool is_openai_mode)
{
    TIMEIT()
    bool ret = true;
    response.clear();

    auto start_tm = chrono::steady_clock::now();
    InferFlowResponseChunk chunk;
    inferService.core_.HandleRequest_Inner(nullptr, &chunk, request, false);
    auto tm = chrono::steady_clock::now();

    int time_cost = (int)chrono::duration_cast<chrono::milliseconds>(tm - start_tm).count();
    chunk.time_cost = time_cost / 1000.0f;

    if (chunk.ret_code.empty()) {
        chunk.ret_code = ret ? L"succ" : L"error.processing";
    }

    wstringstream json_str_stream;
    json_str_stream << L"{";
    json_str_stream << L"\"id\": \"cmpl\","; 
    json_str_stream << L"\"choices\": ["; 
    json_str_stream << L"{"; 
    json_str_stream << L"\"index\": 0,"; 
    json_str_stream << L"\"text\": ";
    JsonBuilder::AppendFieldValue(json_str_stream, chunk.text);
    json_str_stream << L"}"; 
    json_str_stream << L"]"; 
    json_str_stream << L"}";
    wstring json_str = json_str_stream.str();

    StringUtil::ToUtf8(response, json_str);

    return ret;
}

bool Ollama_Tags::handle(sslib::BaseHttpServer::HttpResponseWriter &writer, const sslib::BaseHttpServer::HttpTask &task)
{
    if (task.request.url.find(this->url) == std::string::npos)
    {
        return false;
    }
    else
    {
        LogStatusInfo(L"query ollama tags");
        sslib::HttpResponse response;

        response.status_code = 200;
        std::string content_type = task.request.type == sslib::HttpRequest::Method::Get
                ? "text/html" : "application/json";
        inferflow::transformer::InferFlowResponseChunk res_chunk;
        const ModelSpec& config = inferService.core_.config().engine_config.models[0];
        string model_name = config.sid;
        ServiceState state = {
            model_name,
            "2023-11-04T14:56:49.277302595-07:00",
            7365960935,
            "9f438cb9cd581fc025612d27f7c1a6669ff83a8bb0ed86c94fcf4c5440555697",
            "gguf",
            "llama",
            "",
            "13B",
            "Q4_0"
        };
        std::wstring service_state = responseServiceState(state);
        int body_len = service_state.size();
        sslib::Socket::RetCode ret_code = writer.WriteHeader(response.header_lines, content_type,
                        response.status_code, true, body_len);
        std::string body_utf8;
        sslib::StringUtil::ToUtf8(body_utf8, service_state);
        if (ret_code == sslib::Socket::RetCode::Success && body_len > 0)
        {
            ret_code = writer.WriteString(body_utf8);
        }
        return ret_code == sslib::Socket::RetCode::Success;
    }
}

std::wstring Ollama_Tags::responseServiceState(ServiceState& healthState)
{
    nlohmann::json models;
    models[0]["name"] = healthState.name;
    models[0]["modified_at"] = healthState.modified_at;
    models[0]["size"] = healthState.size;
    models[0]["digest"] = healthState.digest;
    models[0]["details"]["format"] = healthState.details_format;
    models[0]["details"]["family"] = healthState.details_family;
    if (healthState.details_families == "")
        models[0]["details"]["families"] = nullptr;
    else
        models[0]["details"]["families"] = healthState.details_families;
    models[0]["details"]["parameter_size"] = healthState.details_parameter_size;
    models[0]["details"]["quantization_level"] = healthState.details_quantization_level;
    nlohmann::json root;
    root["models"] = models;
    std::stringstream ss;
    ss << "";
    ss << root;
    ss << endl;
    ss.flush();
    std::string result_json = ss.str();
    return wstring(result_json.begin(), result_json.end());
}

bool Ollama_Generate::handle(sslib::BaseHttpServer::HttpResponseWriter &writer, const sslib::BaseHttpServer::HttpTask &task)
{
    if (task.request.url.find(this->url) == std::string::npos)
    {
        return false;
    }
    else
    {
        bool is_unknown_type = task.request.type == sslib::HttpRequest::Method::Unknown;
        const auto &http_request = task.request;
        sslib::HttpResponse response;
        response.status_code = 200;
        bool is_valid_query = true;
        uint32_t request_hash = 0;
        bool ret = true;
        inferflow::transformer::InferFlowRequest request;
        std::wstring error_msg;
        std::string content_type = task.request.type == sslib::HttpRequest::Method::Get
                ? "text/html" : "application/json";
        
        if ((int)task.request.body.size() > inferflow::transformer::InferFlowService::MAX_REQUEST_LEN)
        {
            error_msg = L"error.too_long_request";
        }
        request_hash = sslib::Hash::HashCodeUInt32(task.request.body);
        std::wstring request_body = sslib::StringUtil::Utf8ToWideStr(task.request.body);
        ret = parseRequest(request, request_body);
        if (!ret)
        {
            response.status_code = 400;
            error_msg = L"error.invalid_request_format";
            is_valid_query = false;
            LogStatusInfo(L"error.invalid_request_format:\t%u\t%ls", request_hash, task.request.body.c_str());
        }
        else
        {   
            std::wstring encoded_request = sslib::JsonBuilder::EncodeString(request_body);
            LogStatusInfo(L"query_start\t%u\t%ls", request_hash, encoded_request.c_str());

            bool be_keep_alive = !is_unknown_type && !task.hdr.is_close;
            

            sslib::Socket::RetCode ret_code = sslib::Socket::RetCode::Success;
            
            std::string response_body;
            if (is_valid_query) {
                // 执行推理
                ret = handleRequest_ProcessQuery(response_body, request, false);
            }

            int body_len = (int)response_body.size();
            ret_code = writer.WriteHeader(response.header_lines, content_type,
                response.status_code, be_keep_alive, body_len);
            if (is_valid_query && ret_code == sslib::Socket::RetCode::Success
                && !response_body.empty())
            {
                ret_code = writer.WriteString(response_body);
            }
            return ret_code == sslib::Socket::RetCode::Success;
        }
        // 错误处理
        std::wstring response_temp;
        std::string response_body;
        inferflow::transformer::InferFlowResponseChunk res_chunk;
        res_chunk.error_text = error_msg;
        res_chunk.ToJson(response_temp);
        // res_chunk.ToJson_OpenAI(response_temp);
        int body_len = (int)response_temp.size();
        sslib::Socket::RetCode ret_code = writer.WriteHeader(response.header_lines, content_type,
                        response.status_code, true, body_len);
        sslib::StringUtil::ToUtf8(response_body, response_temp);
        ret_code = writer.WriteString(response_body);
        return ret_code == sslib::Socket::RetCode::Success;
    }
}

bool Ollama_Generate::parseRequest(InferFlowRequest &request,
    const wstring &request_str)
{
    

    JsonDoc jdoc;
    JsonObject jobj;
    bool ret;
    inferService.core_.json_parsr_lock_.lock(); //lock the json parser
    try
    {
        ret = inferService.core_.json_parser_.Parse(jdoc, request_str);
        inferService.core_.json_parsr_lock_.unlock(); //unlock the json parser
        Macro_RetFalseIf(!ret);
        jdoc.GetJObject(jobj);
    }
    catch(const std::exception& e)
    {
        inferService.core_.json_parsr_lock_.unlock(); //unlock the json parser
        std::cerr << e.what() << '\n';
    }

    // parse to request instance
    request.Clear();
    //header
    JsonObject header_obj;
    jobj.GetFieldValue(header_obj, L"header", jdoc);
    ret = request.header.FromJson(header_obj, jdoc);

    wstring prompt;
    // JsonArray relevant_snippets_from_changed_files;
    jobj.GetFieldValue(prompt, L"prompt", jdoc);

    size_t prompt_start_pos = 0;
    size_t nextline_end_pos = prompt.find('\n');
    wstring nextline = prompt.substr(0, nextline_end_pos);
    const wchar_t* LANGUAGE_INDICATOR = L"# Language: ";
    size_t language_pos = nextline.find(LANGUAGE_INDICATOR);
    if (language_pos != nextline.npos)
    {
        
        size_t language_str_begin_pos = language_pos + wcslen(LANGUAGE_INDICATOR);
        request.query.code_language = nextline.substr(language_str_begin_pos, nextline_end_pos-language_str_begin_pos);
        prompt_start_pos = nextline_end_pos + 1;
    }

    nextline_end_pos = prompt.find('\n',prompt_start_pos);
    nextline = prompt.substr(prompt_start_pos, nextline_end_pos - prompt_start_pos);
    const wchar_t* PATH_INDICATOR = L"# Path: ";
    size_t path_pos = nextline.find(PATH_INDICATOR);
    if (path_pos != nextline.npos)
    {
        size_t path_str_begin_pos = path_pos + wcslen(PATH_INDICATOR);
        request.query.code_file_path = nextline.substr(path_str_begin_pos, nextline_end_pos-path_str_begin_pos);
        prompt_start_pos = nextline_end_pos + 1;
    }

    const wchar_t* SUFFIX_INDICATOR = L" <SUF> ";
    const wchar_t* MID_INDICATOR = L" <MID>";
    auto suf_idx = prompt.find(SUFFIX_INDICATOR);
    if (suf_idx != prompt.npos)
    {
        // wstring suffix = prompt.substr(0,suf_idx);
        request.query.code_prefix = prompt.substr(prompt_start_pos, suf_idx-prompt_start_pos);
        size_t suffix_begin_idx = suf_idx + wcslen(SUFFIX_INDICATOR);
        auto mid_idx = prompt.find(MID_INDICATOR);
        if (mid_idx != prompt.npos)
            request.query.code_suffix = prompt.substr(suffix_begin_idx, mid_idx - suffix_begin_idx);
        else
            request.query.code_suffix = prompt.substr(suffix_begin_idx);
    }
    else
    {
        request.query.code_prefix = prompt.substr(prompt_start_pos);
    }
    
    
    {
        request.max_output_len = 100;
        request.is_streaming_mode = false;
        request.temperature = 0.01;
        request.random_seed = 1234;
    }

    return true;
}

bool Ollama_Generate::handleRequest_ProcessQuery(string &response,
    const InferFlowRequest &request, bool is_openai_mode)
{
    TIMEIT()
    bool ret = true;
    response.clear();

    auto start_tm = chrono::steady_clock::now();
    InferFlowResponseChunk chunk;
    inferService.core_.HandleRequest_Inner(nullptr, &chunk, request, false);
    auto tm = chrono::steady_clock::now();

    int time_cost = (int)chrono::duration_cast<chrono::milliseconds>(tm - start_tm).count();
    chunk.time_cost = time_cost / 1000.0f;

    if (chunk.ret_code.empty()) {
        chunk.ret_code = ret ? L"succ" : L"error.processing";
    }

    // stringstream ss;
    // nlohmann::json root;
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(now, "%Y-%m-%dT%H:%M:%S");
    string time_str = oss.str();

    wstringstream json_str_stream;
    json_str_stream << L"{";
    json_str_stream << L"\"model\": \"default\","; 
    json_str_stream << L"\"created_at\": " ;
    JsonBuilder::AppendFieldValue(json_str_stream, wstring(time_str.begin(),time_str.end()));
    json_str_stream << L","; 
    json_str_stream << L"\"response\": " ;
    JsonBuilder::AppendFieldValue(json_str_stream, chunk.text);
    json_str_stream << L",";
    json_str_stream << L"\"input_tokens\": " << chunk.input_token_count << L",";
    json_str_stream << L"\"output_tokens\": " << chunk.output_token_count << L",";
    json_str_stream << L"\"total_duration\": " << chunk.time_cost << L",";
    json_str_stream << L"\"done\": true"; 
    json_str_stream << L"}";
    wstring json_str = json_str_stream.str();

    StringUtil::ToUtf8(response, json_str);

    // root["model"] = "default";
    // root["created_at"] = time_str;
    // root["response"] = string(chunk.text.begin(), chunk.text.end());
    // root["done"] = true;
    // root["context"] = Json::arrayValue;
    // root["total_duration"] = 0;
    // root["load_duration"] = 0;
    // root["prompt_eval_count"] = 0;
    // root["prompt_eval_duration"] = 0;
    // root["eval_count"] = 0;
    // root["eval_duration"] = 0;

    // Json::StreamWriterBuilder writer;
    // writer["indentation"] = "";
    
    // string json_str = Json::writeString(writer, root);
    // wstring w_json_str = wstring(json_str.begin(), json_str.end());

    return ret;
}

TRANSFORMER_END
INFER_FLOW_END