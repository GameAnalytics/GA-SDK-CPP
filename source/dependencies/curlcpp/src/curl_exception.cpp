/**
 * File:   curl_exception.cpp
 * Author: Giuseppe Persico
 */

#include "curl_exception.h"

using curl::curl_exception;
using curl::curlcpp_traceback;

// Need to define the traceback here to separate declaration from definition, or we'll get a linker error.
curlcpp_traceback curl::curl_exception::traceback;

std::mutex curl::curl_exception::tracebackLocker;

// Constructor implementation. Every call will push into the calls stack the function name and the error occurred.
curl_exception::curl_exception(const std::string &error, const std::string &fun_name) {
    curl_exception::tracebackLocker.lock();

    curl_exception::traceback.insert(curl_exception::traceback.begin(),curlcpp_traceback_object(error,fun_name));

    curl_exception::tracebackLocker.unlock();
}

// Implementation of destructor.
curl_exception::~curl_exception() NOEXCEPT {
    // ... nothing to do here ...
}
