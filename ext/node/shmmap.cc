/* Copyright (C) by chosen0ne */

#include <node.h>
#include <v8.h>
#include <stdarg.h>
#include "shm_map.h"

using namespace v8;

static void node_shmmap_log(shmmap_log_level level, const char *fmt, ...);
static void node_key_iter(const char *k, const char *v);

static Local<Function> log_cb;
static Local<Function> iter_cb;

Handle<Value> init(const Arguments &arg){
	HandleScope scope;

	int capacity = (int)arg[0]->Int32Value();
	int mem_size = (int)arg[1]->Int32Value();
	Local<String> data_file = arg[2]->ToString();
	char *dat = new char[data_file->Length() + 1];
	data_file->WriteAscii(dat);
	log_cb = Local<Function>::Cast(arg[3]);
	bool success = map_init(capacity, mem_size, dat, node_shmmap_log);
	delete[] dat;

	return scope.Close(Boolean::New(success));
}

Handle<Value> put(const Arguments &arg){
	HandleScope scope;

	Local<String> k = arg[0]->ToString();
	Local<String> v = arg[1]->ToString();
	char *key = new char[k->Length()+1];
	char *val = new char[v->Length()+1];
	k->WriteAscii(key);
	v->WriteAscii(val);

	const char *oldVal = map_put(key, val);
	delete[] key;
	delete[] val;
	if(oldVal == NULL)
		return scope.Close(Undefined());
	return scope.Close(String::New(oldVal));
}

Handle<Value> get(const Arguments &arg){
	HandleScope scope;

	Local<String> k = arg[0]->ToString();
	char *key = new char[k->Length()+1];
	k->WriteAscii(key);
	const char *val = map_get(key);
	delete[] key;
	if(val == NULL)
		return scope.Close(Undefined());
	return scope.Close(String::New(val));
}

Handle<Value> map_contains(const Arguments& args) {
	HandleScope scope;

	Local<String> k = args[0]->ToString();
	char *key = new char[k->Length() + 1];
	k->WriteAscii(key);
	bool exists = map_contains(key);
	delete[] key;
	return scope.Close(Boolean::New(exists));
}

Handle<Value> map_size(const Arguments& args) {
	HandleScope scope;
	Local<Number> _size = Number::New(map_size());
	return scope.Close(_size);
}

Handle<Value> map_iter(const Arguments& args) {
	HandleScope scope;

	iter_cb = Local<Function>::Cast(args[0]);
	map_iter(node_key_iter);
	return scope.Close(Undefined());
}

Handle<Value> map_info(const Arguments& args) {
	HandleScope scope;

	M_mem_info info;
	m_memory_info(&info);
	Local<Object> mem_info = Object::New();
	mem_info->Set(String::NewSymbol("pool_size"), Number::New(info.pool_size));
	mem_info->Set(String::NewSymbol("free_area_size"), Number::New(info.free_area_size));
	mem_info->Set(String::NewSymbol("allocated_area_size"), Number::New(info.allocated_area_size));
	mem_info->Set(String::NewSymbol("used_size"), Number::New(info.real_used_size));
	mem_info->Set(String::NewSymbol("free_size"), Number::New(info.allocated_area_free_size + info.free_area_size));

	return scope.Close(mem_info);
}

void initilizer(Handle<Object> target){
	target->Set(String::NewSymbol("init"),
			FunctionTemplate::New(init)->GetFunction());
	target->Set(String::NewSymbol("put"),
			FunctionTemplate::New(put)->GetFunction());
	target->Set(String::NewSymbol("get"),
			FunctionTemplate::New(get)->GetFunction());
	target->Set(String::NewSymbol("size"),
			FunctionTemplate::New(map_size)->GetFunction());
	target->Set(String::NewSymbol("contains"),
			FunctionTemplate::New(map_contains)->GetFunction());
	target->Set(String::NewSymbol("iter"),
			FunctionTemplate::New(map_iter)->GetFunction());
	target->Set(String::NewSymbol("info"),
			FunctionTemplate::New(map_info)->GetFunction());
}

static void node_shmmap_log(shmmap_log_level level, const char *fmt, ...){
	va_list 		ap;
	char 			*msg = new char[100];
	const unsigned 	argc = 2;

	va_start(ap, fmt);
	vsprintf(msg, fmt, ap);
	Local<Value> argv[argc] = { Local<Value>::New(Number::New(level)), Local<Value>::New(String::New(msg)) };
	log_cb->Call(Context::GetCurrent()->Global(), argc, argv);
	delete[] msg;
}

static void node_key_iter(const char *k, const char *v){
	const unsigned	argc = 2;
	Local<Value> argv[argc] = { Local<Value>::New(String::New(k)), Local<Value>::New(String::New(v)) };
	iter_cb->Call(Context::GetCurrent()->Global(), argc, argv);
}

NODE_MODULE(shmmap, initilizer);
