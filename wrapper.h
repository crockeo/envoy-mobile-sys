// Copied from envoy-mobile (https://github.com/envoyproxy/envoy-mobile).
// Which is licensed under Apache 2.0. Code copyright
// the Envoy Project Authors.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// NOLINT(namespace-envoy)

/**
 * Handle to an Envoy engine instance. Valid only for the lifetime of the engine and not intended
 * for any external interpretation or use.
 */
typedef intptr_t envoy_engine_t;

/**
 * Handle to an outstanding Envoy HTTP stream. Valid only for the duration of the stream and not
 * intended for any external interpretation or use.
 */
typedef intptr_t envoy_stream_t;

/**
 * Result codes returned by all calls made to this interface.
 */
typedef enum {
  ENVOY_SUCCESS = 0,
  ENVOY_FAILURE = 1,
} envoy_status_t;

typedef enum {
  UNSPECIFIED = 0, // Measured quantity does not require a unit, e.g. "items".
  BYTES = 1,
  MICROSECONDS = 2,
  MILLISECONDS = 3,
} envoy_histogram_stat_unit_t;

/**
 * Equivalent constants to envoy_status_t, for contexts where the enum may not be usable.
 */
extern const int kEnvoySuccess;
extern const int kEnvoyFailure;

/**
 * Error code associated with terminal status of a HTTP stream.
 */
typedef enum {
  ENVOY_UNDEFINED_ERROR,
  ENVOY_STREAM_RESET,
  ENVOY_CONNECTION_FAILURE,
  ENVOY_BUFFER_LIMIT_EXCEEDED,
  ENVOY_REQUEST_TIMEOUT,
} envoy_error_code_t;

/**
 * Networks classified by last physical link.
 * ENVOY_NET_GENERIC is default and includes cases where network characteristics are unknown.
 * ENVOY_NET_WLAN includes WiFi and other local area wireless networks.
 * ENVOY_NET_WWAN includes all mobile phone networks.
 */
typedef enum {
  ENVOY_NET_GENERIC = 0,
  ENVOY_NET_WLAN = 1,
  ENVOY_NET_WWAN = 2,
} envoy_network_t;

#ifdef __cplusplus
extern "C" { // release function
#endif
/**
 * Callback indicating Envoy has drained the associated buffer.
 */
typedef void (*envoy_release_f)(void* context);

/**
 * No-op callback.
 */
void envoy_noop_release(void* context);

/**
 * Const version of no-op release callback.
 */
void envoy_noop_const_release(const void* context);

#ifdef __cplusplus
} // release function
#endif

/**
 * Holds raw binary data as an array of bytes.
 */
typedef struct {
  size_t length;
  const uint8_t* bytes;
  envoy_release_f release;
  void* context;
} envoy_data;

/**
 * Holds a single key/value pair.
 */
typedef struct {
  envoy_data key;
  envoy_data value;
} envoy_map_entry;

/**
 * Consistent type for dealing with encodable/processable header counts.
 */
typedef int envoy_map_size_t;

/**
 * Holds a map as an array of envoy_map_entry structs.
 */
typedef struct {
  // Number of entries in the array.
  envoy_map_size_t length;
  // Array of map entries.
  envoy_map_entry* entries;
} envoy_map;

// Multiple header values for the same header key are supported via a comma-delimited string.
typedef envoy_map envoy_headers;

typedef envoy_map envoy_stats_tags;

#ifdef __cplusplus
extern "C" { // utility functions
#endif

/**
 * malloc wrapper that asserts that the returned pointer is valid. Otherwise, the program exits.
 * @param size, the size of memory to be allocated in bytes.
 * @return void*, pointer to the allocated memory.
 */
void* safe_malloc(size_t size);

/**
 * calloc wrapper that asserts that the returned pointer is valid. Otherwise, the program exits.
 * @param count, the number of elements to be allocated.
 * @param size, the size of elements in bytes.
 * @return void*, pointer to the allocated memory.
 */
void* safe_calloc(size_t count, size_t size);

/**
 * Helper function to free/release memory associated with underlying headers.
 * @param headers, envoy_headers to release.
 */
void release_envoy_headers(envoy_headers headers);

/**
 * Helper function to free/release memory associated with underlying stats_tags.
 * @param headers, envoy_stats_tags to release.
 */
void release_envoy_stats_tags(envoy_stats_tags stats_tags);

/**
 * Helper function to copy envoy_headers.
 * @param src, the envoy_headers to copy from.
 * @param envoy_headers, copied headers.
 */
envoy_headers copy_envoy_headers(envoy_headers src);

/**
 * Helper function to copy envoy_data.
 * @param src, the envoy_data to copy from.
 * @return envoy_data, the envoy_data copied from the src.
 */
envoy_data copy_envoy_data(envoy_data src);

#ifdef __cplusplus
} // utility functions
#endif

// Convenience constant to pass to function calls with no data.
// For example when sending a headers-only request.
extern const envoy_data envoy_nodata;

// Convenience constant to pass to function calls with no headers.
extern const envoy_headers envoy_noheaders;

// Convenience constant to pass to function calls with no tags.
extern const envoy_stats_tags envoy_stats_notags;

/*
 * Error struct.
 */
typedef struct {
  envoy_error_code_t error_code;
  envoy_data message;
  // the number of times an operation was attempted before firing this error.
  // For instance this is used in envoy_on_error_f to account for the number of upstream requests
  // made in a retry series before the on error callback fired.
  // -1 is used in scenarios where it does not make sense to have an attempt count for an error.
  // This is different from 0, which intentionally conveys that the action was _not_ executed.
  int32_t attempt_count;
} envoy_error;

#ifdef __cplusplus
extern "C" { // function pointers
#endif

/**
 * Callback signature for headers on an HTTP stream.
 *
 * @param headers, the headers received.
 * @param end_stream, whether the response is headers-only.
 * @param context, contains the necessary state to carry out platform-specific dispatch and
 * execution.
 * @return void*, return context (may be unused).
 */
typedef void* (*envoy_on_headers_f)(envoy_headers headers, bool end_stream, void* context);

/**
 * Callback signature for data on an HTTP stream.
 *
 * This callback can be invoked multiple times when data is streamed.
 *
 * @param data, the data received.
 * @param end_stream, whether the data is the last data frame.
 * @param context, contains the necessary state to carry out platform-specific dispatch and
 * execution.
 * @return void*, return context (may be unused).
 */
typedef void* (*envoy_on_data_f)(envoy_data data, bool end_stream, void* context);

/**
 * Callback signature for metadata on an HTTP stream.
 *
 * Note that metadata frames are prohibited from ending a stream.
 *
 * @param metadata, the metadata received.
 * @param context, contains the necessary state to carry out platform-specific dispatch and
 * execution.
 * @return void*, return context (may be unused).
 */
typedef void* (*envoy_on_metadata_f)(envoy_headers metadata, void* context);

/**
 * Callback signature for trailers on an HTTP stream.
 *
 * Note that end stream is implied when on_trailers is called.
 *
 * @param trailers, the trailers received.
 * @param context, contains the necessary state to carry out platform-specific dispatch and
 * execution.
 * @return void*, return context (may be unused).
 */
typedef void* (*envoy_on_trailers_f)(envoy_headers trailers, void* context);

/**
 * Callback signature for errors with an HTTP stream.
 *
 * This is a TERMINAL callback. Exactly one terminal callback will be called per stream.
 *
 * @param envoy_error, the error received/caused by the async HTTP stream.
 * @param context, contains the necessary state to carry out platform-specific dispatch and
 * execution.
 * @return void*, return context (may be unused).
 */
typedef void* (*envoy_on_error_f)(envoy_error error, void* context);

/**
 * Callback signature for when an HTTP stream bi-directionally completes without error.
 *
 * This is a TERMINAL callback. Exactly one terminal callback will be called per stream.
 *
 * @param context, contains the necessary state to carry out platform-specific dispatch and
 * execution.
 * @return void*, return context (may be unused).
 */
typedef void* (*envoy_on_complete_f)(void* context);

/**
 * Callback signature for when an HTTP stream is cancelled.
 *
 * This is a TERMINAL callback. Exactly one terminal callback will be called per stream.
 *
 * @param context, contains the necessary state to carry out platform-specific dispatch and
 * execution.
 * @return void*, return context (may be unused).
 */
typedef void* (*envoy_on_cancel_f)(void* context);

/**
 * Called when the envoy engine is exiting.
 */
typedef void (*envoy_on_exit_f)(void* context);

/**
 * Called when the envoy has finished its async setup and returned post-init callbacks.
 *
 * @param context, contains the necessary state to carry out platform-specific dispatch and
 * execution.
 */
typedef void (*envoy_on_engine_running_f)(void* context);

/**
 * Called when envoy's logger logs data.
 *
 * @param data, the logged data.
 * @param context, contains the necessary state to carry out platform-specific dispatch and
 * execution.
 */
typedef void (*envoy_logger_log_f)(envoy_data data, const void* context);

/**
 * Called when Envoy is done with the logger.
 *
 * @param context, contains the necessary state to carry out platform-specific dispatch and
 * execution.
 */
typedef void (*envoy_logger_release_f)(const void* context);

#ifdef __cplusplus
} // function pointers
#endif

/**
 * Interface to handle HTTP callbacks.
 */
typedef struct {
  envoy_on_headers_f on_headers;
  envoy_on_data_f on_data;
  envoy_on_metadata_f on_metadata;
  envoy_on_trailers_f on_trailers;
  envoy_on_error_f on_error;
  envoy_on_complete_f on_complete;
  envoy_on_cancel_f on_cancel;
  // Context passed through to callbacks to provide dispatch and execution state.
  void* context;
} envoy_http_callbacks;

/**
 * Interface that can handle engine callbacks.
 */
typedef struct {
  envoy_on_engine_running_f on_engine_running;
  envoy_on_exit_f on_exit;
  // Context passed through to callbacks to provide dispatch and execution state.
  void* context;
} envoy_engine_callbacks;

/**
 * Interface for logging.
 */
typedef struct {
  envoy_logger_log_f log;
  envoy_logger_release_f release;
  // Context passed through to callbacks to provide dispatch and execution state.
  const void* context;
} envoy_logger;
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// NOLINT(namespace-envoy)

#ifdef __cplusplus
extern "C" { // functions
#endif

/**
 * Template configuration compiled with the Envoy Mobile library.
 * More information about Envoy's config can be found at:
 * https://www.envoyproxy.io/docs/envoy/latest/configuration/configuration
 */
extern const char* config_template;

/**
 * Template configuration used for dynamic creation of the platform-bridged filter chain.
 */
extern const char* platform_filter_template;

/**
 * Template configuration used for dynamic creation of the native filter chain.
 */
extern const char* native_filter_template;

/**
 * Template that enables the route cache reset filter in the chain.
 * Should only be added when the route cache should be cleared on every request
 * going through the filter chain between initial route resolution and the router
 * filter's invocation on the request path. Typically only used for enabling
 * direct responses to mutate headers which are then later used for routing.
 */
extern const char* route_cache_reset_filter_template;

/**
 * Template configuration used for creating "fake" remote clusters which enable
 * local responses to be returned via direct response configurations.
 */
extern const char* fake_remote_cluster_template;

/**
 * Template configuration used for creating "fake" remote listeners which enable
 * local responses to be returned via direct response configurations.
 */
extern const char* fake_remote_listener_template;

/**
 * Template used for setting up the stats sink.
 */
extern const char* stats_sink_template;

/**
 * Initialize an underlying HTTP stream.
 * @param engine, handle to the engine that will manage this stream.
 * @return envoy_stream_t, handle to the underlying stream.
 */
envoy_stream_t init_stream(envoy_engine_t engine);

/**
 * Open an underlying HTTP stream. Note: Streams must be started before other other interaction can
 * can occur.
 * @param stream, handle to the stream to be started.
 * @param callbacks, the callbacks that will run the stream callbacks.
 * @return envoy_stream, with a stream handle and a success status, or a failure status.
 */
envoy_status_t start_stream(envoy_stream_t stream, envoy_http_callbacks callbacks);

/**
 * Send headers over an open HTTP stream. This method can be invoked once and needs to be called
 * before send_data.
 * @param stream, the stream to send headers over.
 * @param headers, the headers to send.
 * @param end_stream, supplies whether this is headers only.
 * @return envoy_status_t, the resulting status of the operation.
 */
envoy_status_t send_headers(envoy_stream_t stream, envoy_headers headers, bool end_stream);

/**
 * Send data over an open HTTP stream. This method can be invoked multiple times.
 * @param stream, the stream to send data over.
 * @param data, the data to send.
 * @param end_stream, supplies whether this is the last data in the stream.
 * @return envoy_status_t, the resulting status of the operation.
 */
envoy_status_t send_data(envoy_stream_t stream, envoy_data data, bool end_stream);

/**
 * Send metadata over an HTTP stream. This method can be invoked multiple times.
 * @param stream, the stream to send metadata over.
 * @param metadata, the metadata to send.
 * @return envoy_status_t, the resulting status of the operation.
 */
envoy_status_t send_metadata(envoy_stream_t stream, envoy_headers metadata);

/**
 * Send trailers over an open HTTP stream. This method can only be invoked once per stream.
 * Note that this method implicitly ends the stream.
 * @param stream, the stream to send trailers over.
 * @param trailers, the trailers to send.
 * @return envoy_status_t, the resulting status of the operation.
 */
envoy_status_t send_trailers(envoy_stream_t stream, envoy_headers trailers);

/**
 * Detach all callbacks from a stream and send an interrupt upstream if supported by transport.
 * @param stream, the stream to evict.
 * @return envoy_status_t, the resulting status of the operation.
 */
envoy_status_t reset_stream(envoy_stream_t stream);

/**
 * Update the network interface to the preferred network for opening new streams.
 * Note that this state is shared by all engines.
 * @param network, the network to be preferred for new streams.
 * @return envoy_status_t, the resulting status of the operation.
 */
envoy_status_t set_preferred_network(envoy_network_t network);

/**
 * Increment a counter with the given elements and by the given count.
 * @param engine, the engine that owns the counter.
 * @param elements, the string that identifies the counter to increment.
 * @param tags, a map of {key, value} pairs of tags.
 * @param count, the count to increment by.
 */
envoy_status_t record_counter_inc(envoy_engine_t, const char* elements, envoy_stats_tags tags,
                                  uint64_t count);

/**
 * Set a gauge of a given string of elements with the given value.
 * @param engine, the engine that owns the gauge.
 * @param elements, the string that identifies the gauge to set value with.
 * @param tags, a map of {key, value} pairs of tags.
 * @param value, the value to set to the gauge.
 */
envoy_status_t record_gauge_set(envoy_engine_t engine, const char* elements, envoy_stats_tags tags,
                                uint64_t value);

/**
 * Add the gauge with the given string of elements and by the given amount.
 * @param engine, the engine that owns the gauge.
 * @param elements, the string that identifies the gauge to add to.
 * @param tags, a map of {key, value} pairs of tags.
 * @param amount, the amount to add to the gauge.
 */
envoy_status_t record_gauge_add(envoy_engine_t engine, const char* elements, envoy_stats_tags tags,
                                uint64_t amount);

/**
 * Subtract from the gauge with the given string of elements and by the given amount.
 * @param engine, the engine that owns the gauge.
 * @param elements, the string that identifies the gauge to subtract from.
 * @param tags, a map of {key, value} pairs of tags.
 * @param amount, amount to subtract from the gauge.
 */
envoy_status_t record_gauge_sub(envoy_engine_t engine, const char* elements, envoy_stats_tags tags,
                                uint64_t amount);

/**
 * Add another recorded amount to the histogram with the given string of elements and unit
 * measurement.
 * @param engine, the engine that owns the histogram.
 * @param elements, the string that identifies the histogram to subtract from.
 * @param tags, a map of {key, value} pairs of tags.
 * @param value, amount to record as a new value for the histogram distribution.
 * @param unit_measure, the unit of measurement (e.g. milliseconds, bytes, etc.)
 */
envoy_status_t record_histogram_value(envoy_engine_t engine, const char* elements,
                                      envoy_stats_tags tags, uint64_t value,
                                      envoy_histogram_stat_unit_t unit_measure);

/**
 * Statically register APIs leveraging platform libraries.
 * Warning: Must be completed before any calls to run_engine().
 * @param name, identifier of the platform API
 * @param api, type-erased c struct containing function pointers and context.
 * @return envoy_status_t, the resulting status of the operation.
 */
envoy_status_t register_platform_api(const char* name, void* api);

/**
 * Initialize an engine for handling network streams.
 * @param callbacks, the callbacks that will run the engine callbacks.
 * @param logger, optional callbacks to handle logging.
 * @return envoy_engine_t, handle to the underlying engine.
 */
envoy_engine_t init_engine(envoy_engine_callbacks callbacks, envoy_logger logger);

/**
 * External entry point for library.
 * @param engine, handle to the engine to run.
 * @param config, the configuration blob to run envoy with.
 * @param log_level, the logging level to run envoy with.
 * @return envoy_status_t, the resulting status of the operation.
 */
envoy_status_t run_engine(envoy_engine_t engine, const char* config, const char* log_level);

void terminate_engine(envoy_engine_t engine);

#ifdef __cplusplus
} // functions
#endif
