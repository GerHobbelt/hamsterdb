/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

#ifndef PROTOBUF_C_messages_2eproto__INCLUDED
#define PROTOBUF_C_messages_2eproto__INCLUDED

#include <google/protobuf-c/protobuf-c.h>

PROTOBUF_C_BEGIN_DECLS


typedef struct _Ham__Wrapper Ham__Wrapper;
typedef struct _Ham__ConnectRequest Ham__ConnectRequest;
typedef struct _Ham__ConnectReply Ham__ConnectReply;
typedef struct _Ham__EnvGetParametersRequest Ham__EnvGetParametersRequest;
typedef struct _Ham__EnvGetParametersReply Ham__EnvGetParametersReply;
typedef struct _Ham__EnvGetDatabaseNamesRequest Ham__EnvGetDatabaseNamesRequest;
typedef struct _Ham__EnvGetDatabaseNamesReply Ham__EnvGetDatabaseNamesReply;
typedef struct _Ham__EnvRenameRequest Ham__EnvRenameRequest;
typedef struct _Ham__EnvRenameReply Ham__EnvRenameReply;
typedef struct _Ham__EnvFlushRequest Ham__EnvFlushRequest;
typedef struct _Ham__EnvFlushReply Ham__EnvFlushReply;


/* --- enums --- */

typedef enum _Ham__Wrapper__Type {
  HAM__WRAPPER__TYPE__CONNECT_REQUEST = 10,
  HAM__WRAPPER__TYPE__CONNECT_REPLY = 11,
  HAM__WRAPPER__TYPE__ENV_RENAME_REQUEST = 20,
  HAM__WRAPPER__TYPE__ENV_RENAME_REPLY = 21,
  HAM__WRAPPER__TYPE__ENV_GET_PARAMETERS_REQUEST = 30,
  HAM__WRAPPER__TYPE__ENV_GET_PARAMETERS_REPLY = 31,
  HAM__WRAPPER__TYPE__ENV_GET_DATABASE_NAMES_REQUEST = 40,
  HAM__WRAPPER__TYPE__ENV_GET_DATABASE_NAMES_REPLY = 41,
  HAM__WRAPPER__TYPE__ENV_FLUSH_REQUEST = 50,
  HAM__WRAPPER__TYPE__ENV_FLUSH_REPLY = 51
} Ham__Wrapper__Type;

/* --- messages --- */

struct  _Ham__Wrapper
{
  ProtobufCMessage base;
  Ham__Wrapper__Type type;
  Ham__ConnectRequest *connect_request;
  Ham__ConnectReply *connect_reply;
  Ham__EnvRenameRequest *env_rename_request;
  Ham__EnvRenameReply *env_rename_reply;
  Ham__EnvGetParametersRequest *env_get_parameters_request;
  Ham__EnvGetParametersReply *env_get_parameters_reply;
  Ham__EnvGetDatabaseNamesRequest *env_get_database_names_request;
  Ham__EnvGetDatabaseNamesReply *env_get_database_names_reply;
  Ham__EnvFlushRequest *env_flush_request;
  Ham__EnvFlushReply *env_flush_reply;
};
#define HAM__WRAPPER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ham__wrapper__descriptor) \
    , 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }


struct  _Ham__ConnectRequest
{
  ProtobufCMessage base;
  uint64_t id;
  char *path;
};
#define HAM__CONNECT_REQUEST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ham__connect_request__descriptor) \
    , 0, NULL }


struct  _Ham__ConnectReply
{
  ProtobufCMessage base;
  uint64_t id;
  int32_t status;
};
#define HAM__CONNECT_REPLY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ham__connect_reply__descriptor) \
    , 0, 0 }


struct  _Ham__EnvGetParametersRequest
{
  ProtobufCMessage base;
  uint64_t id;
  size_t n_names;
  uint32_t *names;
};
#define HAM__ENV_GET_PARAMETERS_REQUEST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ham__env_get_parameters_request__descriptor) \
    , 0, 0,NULL }


struct  _Ham__EnvGetParametersReply
{
  ProtobufCMessage base;
  uint64_t id;
  int32_t status;
  protobuf_c_boolean has_cachesize;
  uint32_t cachesize;
  protobuf_c_boolean has_pagesize;
  uint32_t pagesize;
  protobuf_c_boolean has_max_env_databases;
  uint32_t max_env_databases;
  protobuf_c_boolean has_flags;
  uint32_t flags;
  protobuf_c_boolean has_filemode;
  uint32_t filemode;
  char *filename;
};
#define HAM__ENV_GET_PARAMETERS_REPLY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ham__env_get_parameters_reply__descriptor) \
    , 0, 0, 0,0, 0,0, 0,0, 0,0, 0,0, NULL }


struct  _Ham__EnvGetDatabaseNamesRequest
{
  ProtobufCMessage base;
  uint64_t id;
};
#define HAM__ENV_GET_DATABASE_NAMES_REQUEST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ham__env_get_database_names_request__descriptor) \
    , 0 }


struct  _Ham__EnvGetDatabaseNamesReply
{
  ProtobufCMessage base;
  uint64_t id;
  int32_t status;
  size_t n_names;
  uint32_t *names;
};
#define HAM__ENV_GET_DATABASE_NAMES_REPLY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ham__env_get_database_names_reply__descriptor) \
    , 0, 0, 0,NULL }


struct  _Ham__EnvRenameRequest
{
  ProtobufCMessage base;
  uint64_t id;
  uint32_t oldname;
  uint32_t newname;
  uint32_t flags;
};
#define HAM__ENV_RENAME_REQUEST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ham__env_rename_request__descriptor) \
    , 0, 0, 0, 0 }


struct  _Ham__EnvRenameReply
{
  ProtobufCMessage base;
  uint64_t id;
  int32_t status;
};
#define HAM__ENV_RENAME_REPLY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ham__env_rename_reply__descriptor) \
    , 0, 0 }


struct  _Ham__EnvFlushRequest
{
  ProtobufCMessage base;
  uint64_t id;
  uint32_t flags;
};
#define HAM__ENV_FLUSH_REQUEST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ham__env_flush_request__descriptor) \
    , 0, 0 }


struct  _Ham__EnvFlushReply
{
  ProtobufCMessage base;
  uint64_t id;
  int32_t status;
};
#define HAM__ENV_FLUSH_REPLY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ham__env_flush_reply__descriptor) \
    , 0, 0 }


/* Ham__Wrapper methods */
void   ham__wrapper__init
                     (Ham__Wrapper         *message);
size_t ham__wrapper__get_packed_size
                     (const Ham__Wrapper   *message);
size_t ham__wrapper__pack
                     (const Ham__Wrapper   *message,
                      uint8_t             *out);
size_t ham__wrapper__pack_to_buffer
                     (const Ham__Wrapper   *message,
                      ProtobufCBuffer     *buffer);
Ham__Wrapper *
       ham__wrapper__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ham__wrapper__free_unpacked
                     (Ham__Wrapper *message,
                      ProtobufCAllocator *allocator);
/* Ham__ConnectRequest methods */
void   ham__connect_request__init
                     (Ham__ConnectRequest         *message);
size_t ham__connect_request__get_packed_size
                     (const Ham__ConnectRequest   *message);
size_t ham__connect_request__pack
                     (const Ham__ConnectRequest   *message,
                      uint8_t             *out);
size_t ham__connect_request__pack_to_buffer
                     (const Ham__ConnectRequest   *message,
                      ProtobufCBuffer     *buffer);
Ham__ConnectRequest *
       ham__connect_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ham__connect_request__free_unpacked
                     (Ham__ConnectRequest *message,
                      ProtobufCAllocator *allocator);
/* Ham__ConnectReply methods */
void   ham__connect_reply__init
                     (Ham__ConnectReply         *message);
size_t ham__connect_reply__get_packed_size
                     (const Ham__ConnectReply   *message);
size_t ham__connect_reply__pack
                     (const Ham__ConnectReply   *message,
                      uint8_t             *out);
size_t ham__connect_reply__pack_to_buffer
                     (const Ham__ConnectReply   *message,
                      ProtobufCBuffer     *buffer);
Ham__ConnectReply *
       ham__connect_reply__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ham__connect_reply__free_unpacked
                     (Ham__ConnectReply *message,
                      ProtobufCAllocator *allocator);
/* Ham__EnvGetParametersRequest methods */
void   ham__env_get_parameters_request__init
                     (Ham__EnvGetParametersRequest         *message);
size_t ham__env_get_parameters_request__get_packed_size
                     (const Ham__EnvGetParametersRequest   *message);
size_t ham__env_get_parameters_request__pack
                     (const Ham__EnvGetParametersRequest   *message,
                      uint8_t             *out);
size_t ham__env_get_parameters_request__pack_to_buffer
                     (const Ham__EnvGetParametersRequest   *message,
                      ProtobufCBuffer     *buffer);
Ham__EnvGetParametersRequest *
       ham__env_get_parameters_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ham__env_get_parameters_request__free_unpacked
                     (Ham__EnvGetParametersRequest *message,
                      ProtobufCAllocator *allocator);
/* Ham__EnvGetParametersReply methods */
void   ham__env_get_parameters_reply__init
                     (Ham__EnvGetParametersReply         *message);
size_t ham__env_get_parameters_reply__get_packed_size
                     (const Ham__EnvGetParametersReply   *message);
size_t ham__env_get_parameters_reply__pack
                     (const Ham__EnvGetParametersReply   *message,
                      uint8_t             *out);
size_t ham__env_get_parameters_reply__pack_to_buffer
                     (const Ham__EnvGetParametersReply   *message,
                      ProtobufCBuffer     *buffer);
Ham__EnvGetParametersReply *
       ham__env_get_parameters_reply__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ham__env_get_parameters_reply__free_unpacked
                     (Ham__EnvGetParametersReply *message,
                      ProtobufCAllocator *allocator);
/* Ham__EnvGetDatabaseNamesRequest methods */
void   ham__env_get_database_names_request__init
                     (Ham__EnvGetDatabaseNamesRequest         *message);
size_t ham__env_get_database_names_request__get_packed_size
                     (const Ham__EnvGetDatabaseNamesRequest   *message);
size_t ham__env_get_database_names_request__pack
                     (const Ham__EnvGetDatabaseNamesRequest   *message,
                      uint8_t             *out);
size_t ham__env_get_database_names_request__pack_to_buffer
                     (const Ham__EnvGetDatabaseNamesRequest   *message,
                      ProtobufCBuffer     *buffer);
Ham__EnvGetDatabaseNamesRequest *
       ham__env_get_database_names_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ham__env_get_database_names_request__free_unpacked
                     (Ham__EnvGetDatabaseNamesRequest *message,
                      ProtobufCAllocator *allocator);
/* Ham__EnvGetDatabaseNamesReply methods */
void   ham__env_get_database_names_reply__init
                     (Ham__EnvGetDatabaseNamesReply         *message);
size_t ham__env_get_database_names_reply__get_packed_size
                     (const Ham__EnvGetDatabaseNamesReply   *message);
size_t ham__env_get_database_names_reply__pack
                     (const Ham__EnvGetDatabaseNamesReply   *message,
                      uint8_t             *out);
size_t ham__env_get_database_names_reply__pack_to_buffer
                     (const Ham__EnvGetDatabaseNamesReply   *message,
                      ProtobufCBuffer     *buffer);
Ham__EnvGetDatabaseNamesReply *
       ham__env_get_database_names_reply__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ham__env_get_database_names_reply__free_unpacked
                     (Ham__EnvGetDatabaseNamesReply *message,
                      ProtobufCAllocator *allocator);
/* Ham__EnvRenameRequest methods */
void   ham__env_rename_request__init
                     (Ham__EnvRenameRequest         *message);
size_t ham__env_rename_request__get_packed_size
                     (const Ham__EnvRenameRequest   *message);
size_t ham__env_rename_request__pack
                     (const Ham__EnvRenameRequest   *message,
                      uint8_t             *out);
size_t ham__env_rename_request__pack_to_buffer
                     (const Ham__EnvRenameRequest   *message,
                      ProtobufCBuffer     *buffer);
Ham__EnvRenameRequest *
       ham__env_rename_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ham__env_rename_request__free_unpacked
                     (Ham__EnvRenameRequest *message,
                      ProtobufCAllocator *allocator);
/* Ham__EnvRenameReply methods */
void   ham__env_rename_reply__init
                     (Ham__EnvRenameReply         *message);
size_t ham__env_rename_reply__get_packed_size
                     (const Ham__EnvRenameReply   *message);
size_t ham__env_rename_reply__pack
                     (const Ham__EnvRenameReply   *message,
                      uint8_t             *out);
size_t ham__env_rename_reply__pack_to_buffer
                     (const Ham__EnvRenameReply   *message,
                      ProtobufCBuffer     *buffer);
Ham__EnvRenameReply *
       ham__env_rename_reply__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ham__env_rename_reply__free_unpacked
                     (Ham__EnvRenameReply *message,
                      ProtobufCAllocator *allocator);
/* Ham__EnvFlushRequest methods */
void   ham__env_flush_request__init
                     (Ham__EnvFlushRequest         *message);
size_t ham__env_flush_request__get_packed_size
                     (const Ham__EnvFlushRequest   *message);
size_t ham__env_flush_request__pack
                     (const Ham__EnvFlushRequest   *message,
                      uint8_t             *out);
size_t ham__env_flush_request__pack_to_buffer
                     (const Ham__EnvFlushRequest   *message,
                      ProtobufCBuffer     *buffer);
Ham__EnvFlushRequest *
       ham__env_flush_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ham__env_flush_request__free_unpacked
                     (Ham__EnvFlushRequest *message,
                      ProtobufCAllocator *allocator);
/* Ham__EnvFlushReply methods */
void   ham__env_flush_reply__init
                     (Ham__EnvFlushReply         *message);
size_t ham__env_flush_reply__get_packed_size
                     (const Ham__EnvFlushReply   *message);
size_t ham__env_flush_reply__pack
                     (const Ham__EnvFlushReply   *message,
                      uint8_t             *out);
size_t ham__env_flush_reply__pack_to_buffer
                     (const Ham__EnvFlushReply   *message,
                      ProtobufCBuffer     *buffer);
Ham__EnvFlushReply *
       ham__env_flush_reply__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ham__env_flush_reply__free_unpacked
                     (Ham__EnvFlushReply *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Ham__Wrapper_Closure)
                 (const Ham__Wrapper *message,
                  void *closure_data);
typedef void (*Ham__ConnectRequest_Closure)
                 (const Ham__ConnectRequest *message,
                  void *closure_data);
typedef void (*Ham__ConnectReply_Closure)
                 (const Ham__ConnectReply *message,
                  void *closure_data);
typedef void (*Ham__EnvGetParametersRequest_Closure)
                 (const Ham__EnvGetParametersRequest *message,
                  void *closure_data);
typedef void (*Ham__EnvGetParametersReply_Closure)
                 (const Ham__EnvGetParametersReply *message,
                  void *closure_data);
typedef void (*Ham__EnvGetDatabaseNamesRequest_Closure)
                 (const Ham__EnvGetDatabaseNamesRequest *message,
                  void *closure_data);
typedef void (*Ham__EnvGetDatabaseNamesReply_Closure)
                 (const Ham__EnvGetDatabaseNamesReply *message,
                  void *closure_data);
typedef void (*Ham__EnvRenameRequest_Closure)
                 (const Ham__EnvRenameRequest *message,
                  void *closure_data);
typedef void (*Ham__EnvRenameReply_Closure)
                 (const Ham__EnvRenameReply *message,
                  void *closure_data);
typedef void (*Ham__EnvFlushRequest_Closure)
                 (const Ham__EnvFlushRequest *message,
                  void *closure_data);
typedef void (*Ham__EnvFlushReply_Closure)
                 (const Ham__EnvFlushReply *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor ham__wrapper__descriptor;
extern const ProtobufCEnumDescriptor    ham__wrapper__type__descriptor;
extern const ProtobufCMessageDescriptor ham__connect_request__descriptor;
extern const ProtobufCMessageDescriptor ham__connect_reply__descriptor;
extern const ProtobufCMessageDescriptor ham__env_get_parameters_request__descriptor;
extern const ProtobufCMessageDescriptor ham__env_get_parameters_reply__descriptor;
extern const ProtobufCMessageDescriptor ham__env_get_database_names_request__descriptor;
extern const ProtobufCMessageDescriptor ham__env_get_database_names_reply__descriptor;
extern const ProtobufCMessageDescriptor ham__env_rename_request__descriptor;
extern const ProtobufCMessageDescriptor ham__env_rename_reply__descriptor;
extern const ProtobufCMessageDescriptor ham__env_flush_request__descriptor;
extern const ProtobufCMessageDescriptor ham__env_flush_reply__descriptor;

PROTOBUF_C_END_DECLS


#endif  /* PROTOBUF_messages_2eproto__INCLUDED */
