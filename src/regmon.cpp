#include <napi.h>
#include <iostream>
#include <windows.h>

class Watcher {
private:
	HKEY   hKey;

	HKEY strToRootHKey(std::string rootkey) {

		HKEY  hRootKey;

		if ((rootkey.compare("HKLM")) == 0) {
			hRootKey = HKEY_LOCAL_MACHINE;
		}
		else if ((rootkey.compare("HKU")) == 0) {
			hRootKey = HKEY_USERS;
		}
		else if ((rootkey.compare("HKCU")) == 0) {
			hRootKey = HKEY_CURRENT_USER;
		}
		else if ((rootkey.compare("HKCR")) == 0) {
			hRootKey = HKEY_CLASSES_ROOT;
		}
		else if ((rootkey.compare("HKCC")) == 0) {
			hRootKey = HKEY_CURRENT_CONFIG;
		}
		else {
			throw std::invalid_argument("Unknown rootkey shorthand");
		}

		return hRootKey;

	}

public:
  HANDLE hEvent;
  
	Watcher(std::string rootkey, std::string subkey) //Constructor
	{
		this->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		
		LONG   lErrorCode;

		lErrorCode = RegOpenKeyEx(
			this->strToRootHKey(rootkey),
			subkey.c_str(),
			0,
			KEY_NOTIFY | KEY_WOW64_64KEY,
			&this->hKey
		);

		if (lErrorCode != ERROR_SUCCESS)
		{
			throw std::runtime_error("Error opening key");
		}

	}
	
  void RegNotifyChange()
	{

		const DWORD dwEventFilter = REG_NOTIFY_CHANGE_NAME |
									REG_NOTIFY_CHANGE_ATTRIBUTES |
									REG_NOTIFY_CHANGE_LAST_SET |
									REG_NOTIFY_CHANGE_SECURITY;

		LONG   lErrorCode;

		lErrorCode = RegNotifyChangeKeyValue(this->hKey,
			TRUE,
			dwEventFilter,
			this->hEvent,
			TRUE);

		if (lErrorCode != ERROR_SUCCESS)
		{
			throw std::runtime_error("Error monitoring key");
		}

	}
	
	void close() {
		CloseHandle(this->hEvent);
		RegCloseKey(this->hKey);
	}
};

/* Async */

class AsyncWorker: public Napi::AsyncWorker {
  public:
    AsyncWorker(Watcher* watcher, Napi::Function& callback)
      : Napi::AsyncWorker(callback), watcher_ref(watcher) {
    }

  private:
    void Execute() {
    }

    void OnOK() {
    
      Napi::HandleScope scope(Env()); 
      
      try{
        watcher_ref->RegNotifyChange();
        while (WaitForSingleObject(watcher_ref->hEvent, INFINITE) != WAIT_FAILED)
        {
          Callback().Call({
            Env().Undefined()
          });
          watcher_ref->RegNotifyChange();
        }
      }catch(std::exception& e){
        Napi::Error::New(Env(), e.what()).ThrowAsJavaScriptException();
      }
    }
    
    Watcher* watcher_ref;
};

/* NAPI Class wrapper */

class Monitor : public Napi::ObjectWrap<Monitor> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Monitor(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;
  void Watch(const Napi::CallbackInfo& info);
  void Close(const Napi::CallbackInfo& info);
  Watcher *monitor_;
  AsyncWorker* worker;
};

Napi::FunctionReference Monitor::constructor;

Napi::Object Monitor::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Monitor", {
    InstanceMethod("watch", &Monitor::Watch),
    InstanceMethod("close", &Monitor::Close),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Monitor", func);
  return exports;
}

Monitor::Monitor(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Monitor>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  int length = info.Length();
  
  if (length != 2 || !info[0].IsString() || !info[1].IsString()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }

  Napi::String param0 = info[0].As<Napi::String>();
  Napi::String param1 = info[1].As<Napi::String>();
  
  try{
    this->monitor_ = new Watcher(param0,param1);
  }catch(std::exception& e){
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
  }
}

void Monitor::Watch(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  int length = info.Length();
  
  if (info.Length() < 1) {
     throw Napi::TypeError::New(env, "1 argument expected");
  }
  Napi::Function callback = info[0].As<Napi::Function>();
    
  this->worker = new AsyncWorker(this->monitor_,callback);
  this->worker->Queue();
}

void Monitor::Close(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  
  delete this->worker;
  this->monitor_->close();
}

/* NAPI Initialize add-on*/

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  return Monitor::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)