#pragma once

#include "Core/Object.h"
#include "Core/Timer.h"
#include <curl/curl.h>

class Curl : public Object {
public:
	typedef void (*OnHandleDoneCallback)(CURL* handle, CURLcode result, void* arg);
	Curl();
	~Curl();

	void addHandle(CURL* handle, OnHandleDoneCallback onHandleDone, void* arg);
	void removeHandle(CURL* handle);

private:
	struct curl_context_t {
		uv_poll_t poll_handle;
		curl_socket_t sockfd;
		Curl* instance;
	};

	void onTimeout();
	void onCheckCurlInfo();

	curl_context_t* createCurlContext(curl_socket_t sockfd);
	static void destroyCurlContext(curl_context_t* context);
	static void curlCloseCb(uv_handle_t* handle);
	static void curlPerform(uv_poll_t* req, int status, int events);
	static int curlDebugFunction(CURL* handle,       /* the handle/transfer this concerns */
	                             curl_infotype type, /* what kind of data */
	                             char* data,         /* points to the data */
	                             size_t size,        /* size of the data pointed to */
	                             void* userptr);
	static int curlWriteFunction(char* buffer, size_t size, size_t nitems, void* outstream);
	static int curlReadFunction(char* buffer, size_t size, size_t nitems, void* instream);

	static int curlTimerFunctionStatic(CURLM* multi, long timeout_ms, void* userp);
	static int curlOnSocketFunctionStatic(CURL* easy, curl_socket_t s, int action, void* userp, void* socketp);

private:
	struct CurlHandle {
		CURL* handle;
		OnHandleDoneCallback onHandleDone;
		void* arg;
	};
	CURLM* multi_handle;
	Timer<Curl> timer;
};
