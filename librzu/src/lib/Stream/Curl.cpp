#include "Curl.h"
#include "Core/EventLoop.h"

Curl::Curl() {
	multi_handle = curl_multi_init();
	curl_multi_setopt(multi_handle, CURLMOPT_SOCKETFUNCTION, &curlOnSocketFunctionStatic);
	curl_multi_setopt(multi_handle, CURLMOPT_SOCKETDATA, this);
	curl_multi_setopt(multi_handle, CURLMOPT_TIMERFUNCTION, &curlTimerFunctionStatic);
	curl_multi_setopt(multi_handle, CURLMOPT_TIMERDATA, this);
}

Curl::~Curl() {
	curl_multi_cleanup(multi_handle);
}

void Curl::addHandle(CURL* handle, OnHandleDoneCallback onHandleDone, void* arg) {
	CurlHandle* curlHandle = new CurlHandle;

	curlHandle->handle = handle;
	curlHandle->onHandleDone = onHandleDone;
	curlHandle->arg = arg;

	curl_easy_setopt(handle, CURLOPT_PRIVATE, curlHandle);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &Curl::curlWriteFunction);
	curl_easy_setopt(handle, CURLOPT_READFUNCTION, &Curl::curlReadFunction);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(handle, CURLOPT_DEBUGFUNCTION, &Curl::curlDebugFunction);
	curl_easy_setopt(handle, CURLOPT_DEBUGDATA, this);

	curl_multi_add_handle(multi_handle, handle);
}

void Curl::removeHandle(CURL* handle) {
	CurlHandle* curlHandle;

	if(curl_easy_getinfo(handle, CURLINFO_PRIVATE, &curlHandle) == CURLE_OK) {
		curl_easy_setopt(handle, CURLOPT_PRIVATE, nullptr);
		delete curlHandle;
	}

	curl_multi_remove_handle(multi_handle, handle);
}

void Curl::onCheckCurlInfo() {
	char* done_url;
	CURLMsg* message;
	int pending;

	while((message = curl_multi_info_read(multi_handle, &pending))) {
		switch(message->msg) {
			case CURLMSG_DONE: {
				/* Do not use message data after calling curl_multi_remove_handle() and
				 * curl_easy_cleanup(). As per curl_multi_info_read() docs:
				 * "WARNING: The data the returned pointer points to will not survive
				 * calling curl_multi_cleanup, curl_multi_remove_handle or
				 * curl_easy_cleanup."
				 */
				CURL* easy_handle = message->easy_handle;
				CURLcode result = message->data.result;

				curl_easy_getinfo(easy_handle, CURLINFO_EFFECTIVE_URL, &done_url);
				log(LL_Debug, "%s DONE\n", done_url);

				CurlHandle* curlHandle;

				if(curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &curlHandle) == CURLE_OK) {
					curlHandle->onHandleDone(easy_handle, result, curlHandle->arg);
					delete curlHandle;
				}

				curl_multi_remove_handle(multi_handle, easy_handle);
				curl_easy_cleanup(easy_handle);
				break;
			}

			default:
				log(LL_Error, "CURLMSG default\n");
				break;
		}
	}
}

Curl::curl_context_t* Curl::createCurlContext(curl_socket_t sockfd) {
	curl_context_t* context;

	context = (curl_context_t*) malloc(sizeof(*context));

	context->sockfd = sockfd;

	uv_poll_init_socket(EventLoop::getLoop(), &context->poll_handle, sockfd);
	context->poll_handle.data = context;

	context->instance = this;

	return context;
}

void Curl::destroyCurlContext(Curl::curl_context_t* context) {
	uv_close((uv_handle_t*) &context->poll_handle, &curlCloseCb);
}

void Curl::curlCloseCb(uv_handle_t* handle) {
	curl_context_t* context = (curl_context_t*) handle->data;
	free(context);
}

void Curl::onTimeout() {
	int running_handles;
	curl_multi_socket_action(multi_handle, CURL_SOCKET_TIMEOUT, 0, &running_handles);
	onCheckCurlInfo();
}

void Curl::curlPerform(uv_poll_t* req, int status, int events) {
	int running_handles;
	int flags = 0;
	curl_context_t* context;

	if(events & UV_READABLE)
		flags |= CURL_CSELECT_IN;
	if(events & UV_WRITABLE)
		flags |= CURL_CSELECT_OUT;

	context = (curl_context_t*) req->data;

	curl_multi_socket_action(context->instance->multi_handle, context->sockfd, flags, &running_handles);

	context->instance->onCheckCurlInfo();
}

int Curl::curlDebugFunction(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr) {
	Curl* thisInstance = (Curl*) userptr;
	static const char s_infotype[CURLINFO_END][3] = {"* ", "< ", "> ", "{ ", "} ", "{ ", "} "};

	switch(type) {
		case CURLINFO_TEXT:
		case CURLINFO_HEADER_OUT:
		case CURLINFO_HEADER_IN:
			thisInstance->log(LL_Debug, "%s%.*s", s_infotype[type], (int) size, data);
			break;
		default: /* nada */
			break;
	}

	return 0;
}

int Curl::curlWriteFunction(char* buffer, size_t size, size_t nitems, void* outstream) {
	Curl* thisInstance = (Curl*) outstream;
	thisInstance->log(LL_Debug, "%.*s", (int) (size * nitems), buffer);

	return nitems;
}

int Curl::curlReadFunction(char* buffer, size_t size, size_t nitems, void* instream) {
	return 0;
}

int Curl::curlTimerFunctionStatic(CURLM* multi, long timeout_ms, void* userp) {
	Curl* thisInstance = (Curl*) userp;

	if(timeout_ms < 0)
		thisInstance->timer.stop();
	else {
		if(timeout_ms == 0)
			timeout_ms = 1; /* 0 means directly call socket_action, but we'll do it
			             in a bit */
		thisInstance->timer.start(thisInstance, &Curl::onTimeout, timeout_ms, 0);
	}

	return 0;
}

int Curl::curlOnSocketFunctionStatic(CURL* easy, curl_socket_t s, int action, void* userp, void* socketp) {
	Curl* thisInstance = (Curl*) userp;

	curl_context_t* curl_context;
	int events = 0;

	switch(action) {
		case CURL_POLL_IN:
		case CURL_POLL_OUT:
		case CURL_POLL_INOUT:
			curl_context = socketp ? (curl_context_t*) socketp : thisInstance->createCurlContext(s);

			curl_multi_assign(thisInstance->multi_handle, s, (void*) curl_context);

			if(action != CURL_POLL_IN)
				events |= UV_WRITABLE;
			if(action != CURL_POLL_OUT)
				events |= UV_READABLE;

			uv_poll_start(&curl_context->poll_handle, events, &curlPerform);
			break;
		case CURL_POLL_REMOVE:
			if(socketp) {
				uv_poll_stop(&((curl_context_t*) socketp)->poll_handle);
				destroyCurlContext((curl_context_t*) socketp);
				curl_multi_assign(thisInstance->multi_handle, s, NULL);
			}
			break;
		default:
			abort();
	}

	return 0;
}
