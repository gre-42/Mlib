/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "JNIHelper.h"

#include <Mlib/Threads/Thread_Local.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>

#include <cstring>

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

inline size_t stream_off_cast(std::streamoff source) {
    if ((double)std::numeric_limits<std::streamoff>::max() > (double)std::numeric_limits<size_t>::max()) {
        if (source > (std::streamoff)std::numeric_limits<size_t>::max()) {
            THROW_OR_ABORT("Value too large");
        }
    }
    if (source < 0) {
        THROW_OR_ABORT("Value too small");
    }
    return (size_t)source;
}

namespace fs = std::filesystem;

namespace ndk_helper {

#define NATIVEACTIVITY_CLASS_NAME "android/app/NativeActivity"

//---------------------------------------------------------------------------
// JNI Helper functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Singleton
//---------------------------------------------------------------------------
JNIHelper* JNIHelper::GetInstance() {
  static JNIHelper helper;
  return &helper;
}

//---------------------------------------------------------------------------
// Ctor
//---------------------------------------------------------------------------
JNIHelper::JNIHelper() : activity_(nullptr) {}

//---------------------------------------------------------------------------
// Dtor
//---------------------------------------------------------------------------
JNIHelper::~JNIHelper() = default;

void JNIHelper::Destroy() {
    // Lock mutex
    std::scoped_lock lock{mutex_};
    {
        JNIEnv *env = AttachCurrentThread();
        env->DeleteGlobalRef(jni_helper_java_ref_);
        env->DeleteGlobalRef(jni_helper_java_class_);
    }
    DetachCurrentThreadIfNecessary();
}

//---------------------------------------------------------------------------
// Init
//---------------------------------------------------------------------------
void JNIHelper::Init(ANativeActivity* activity, const char* helper_class_name) {
  JNIHelper& helper = *GetInstance();

  helper.activity_ = activity;

  // Lock mutex
  std::scoped_lock lock{helper.mutex_};

  JNIEnv* env = helper.AttachCurrentThread();

  // Retrieve app bundle id
  jclass android_content_Context = env->GetObjectClass(helper.activity_->clazz);
  jmethodID midGetPackageName = env->GetMethodID(
      android_content_Context, "getPackageName", "()Ljava/lang/String;");

  jstring packageName = (jstring)env->CallObjectMethod(helper.activity_->clazz,
                                                       midGetPackageName);
  const char* appname = env->GetStringUTFChars(packageName, NULL);
  helper.app_name_ = std::string(appname);

  jclass cls = helper.RetrieveClass(env, helper_class_name);
  helper.jni_helper_java_class_ = (jclass)env->NewGlobalRef(cls);

  jmethodID constructor =
      env->GetMethodID(helper.jni_helper_java_class_, "<init>",
                       "(Landroid/app/NativeActivity;)V");

  helper.jni_helper_java_ref_ = env->NewObject(helper.jni_helper_java_class_,
                                               constructor, activity->clazz);
  helper.jni_helper_java_ref_ = env->NewGlobalRef(helper.jni_helper_java_ref_);

  // Get app label
  jstring labelName = (jstring)helper.CallObjectMethod("getApplicationName",
                                                       "()Ljava/lang/String;");
  const char* label = env->GetStringUTFChars(labelName, NULL);
  helper.app_label_ = std::string(label);

  env->ReleaseStringUTFChars(packageName, appname);
  env->ReleaseStringUTFChars(labelName, label);
  env->DeleteLocalRef(packageName);
  env->DeleteLocalRef(labelName);
  env->DeleteLocalRef(cls);
}

void JNIHelper::Init(ANativeActivity* activity, const char* helper_class_name,
                     const char* native_soname) {
  Init(activity, helper_class_name);
  if (native_soname) {
    JNIHelper& helper = *GetInstance();
    // Lock mutex
    std::scoped_lock lock{helper.mutex_};

    JNIEnv* env = helper.AttachCurrentThread();

    // Setup soname
    jstring soname = env->NewStringUTF(native_soname);

    jmethodID mid = env->GetMethodID(helper.jni_helper_java_class_,
                                     "loadLibrary", "(Ljava/lang/String;)V");
    env->CallVoidMethod(helper.jni_helper_java_ref_, mid, soname);

    env->DeleteLocalRef(soname);
  }
}

static size_t AssetNameStart(const char* path) {
  size_t start = 0;
  while ((path[start] == '/') || (path[start] == '.')) {
    ++start;
  }
  return start;
}

//---------------------------------------------------------------------------
// readFile
//---------------------------------------------------------------------------
bool JNIHelper::ReadFile(const char* fileName,
                         std::vector<uint8_t>* buffer_ref,
                         StorageType storage_types) {
  if (activity_ == nullptr) {
    Mlib::verbose_abort(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
  }

  for (const auto candidateFileStorage : FILE_STORAGES) {
    if (any(storage_types & candidateFileStorage)) {
      // First, try reading from externalFileDir;
      std::string s = GetFilesDir(candidateFileStorage);
      if (fileName[0] != '/') {
        s.append("/");
      }
      s.append(fileName);
      {
        std::ifstream f(s.c_str(), std::ios::binary);
        if (f) {
          f.seekg(0, std::ifstream::end);
          std::streamoff file_size = f.tellg();
          f.seekg(0, std::ifstream::beg);
          buffer_ref->reserve(stream_off_cast(file_size));
          buffer_ref->assign(std::istreambuf_iterator<char>(f),
                            std::istreambuf_iterator<char>());
          f.close();
          return true;
        }
      }
    }
  }
  if (any(storage_types & StorageType::RESOURCES)) {
    // Fallback to assetManager
    size_t start = AssetNameStart(fileName);
    AAssetManager* assetManager = activity_->assetManager;
    AAsset* assetFile =
        AAssetManager_open(assetManager, fileName + start, AASSET_MODE_BUFFER);
    if (!assetFile) {
      return false;
    }
    auto* data = (uint8_t*)AAsset_getBuffer(assetFile);
    auto size = Mlib::integral_cast<size_t>(AAsset_getLength(assetFile));
    if (data == nullptr) {
      AAsset_close(assetFile);

      LOGE("Failed to load: %s", fileName);
      return false;
    }

    buffer_ref->reserve(size);
    buffer_ref->assign(data, data + size);

    AAsset_close(assetFile);
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
// fileExists
//---------------------------------------------------------------------------
bool JNIHelper::PathExists(
    const char* fileName,
    StorageType storage_types) {
  if (activity_ == nullptr) {
    Mlib::verbose_abort(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
  }

  for (const auto candidateFileStorage : FILE_STORAGES) {
    if (any(storage_types & candidateFileStorage)) {
      // First, try reading from externalFileDir;
      std::string s = GetFilesDir(candidateFileStorage);
      if (fileName[0] != '/') {
        s.append("/");
      }
      s.append(fileName);
      if (fs::exists(s)) {
        return true;
      }
    }
  }
  if (any(storage_types & StorageType::RESOURCES)) {
    // Fallback to assetManager
    size_t start = AssetNameStart(fileName);
    AAssetManager *assetManager = activity_->assetManager;
    {
      AAssetDir *dir = AAssetManager_openDir(assetManager, fileName + start);
      const char *asset_filename = AAssetDir_getNextFileName(dir);
      if (asset_filename != nullptr) {
        AAssetDir_close(dir);
        return true;
      }
    }
    AAsset *assetFile =
      AAssetManager_open(assetManager, fileName + start, AASSET_MODE_STREAMING);
    if (assetFile != nullptr) {
      AAsset_close(assetFile);
      return true;
    }
  }
  return false;
}

DirectoryEntry::DirectoryEntry(
  std::filesystem::path path,
  bool is_listable)
: path_{std::move(path)},
  is_listable_{is_listable}
{}

const std::filesystem::path& DirectoryEntry::path() const {
  return path_;
}

DirectoryEntry::operator const std::filesystem::path& () const {
  return path_;
}

bool DirectoryEntry::is_listable() const {
  return is_listable_;
}

DirectoryIterator JNIHelper::ListDir(const char* dir_name) {
  return DirectoryIterator(activity_->assetManager, dir_name);
}

DirectoryIterator::DirectoryIterator(DirectoryIterator&& other) noexcept = default;

DirectoryIterator::DirectoryIterator()
: asset_dir_{nullptr, AAssetDir_close},
  current_asset_filename_{nullptr}
{}

DirectoryIterator::DirectoryIterator(
    AAssetManager* mgr,
    const char* dir_name)
: dir_name_{dir_name},
  asset_dir_{ AAssetManager_openDir(mgr, dir_name + AssetNameStart(dir_name)), AAssetDir_close },
  current_asset_filename_{ AAssetDir_getNextFileName(asset_dir_.get()) }
{
  for (const auto candidateFileStorage : FILE_STORAGES) {
    if (JNIHelper::GetInstance()->PathExists(dir_name, candidateFileStorage)) {
      std::string s = JNIHelper::GetInstance()->GetFilesDir(candidateFileStorage);
      if (dir_name[0] != '/') {
        s.append("/");
      }
      s.append(dir_name);
      std::error_code ec;
      filesystem_directory_iterators_.emplace_back(fs::directory_iterator(s, ec), candidateFileStorage);
      if (ec) {
        Mlib::verbose_abort(
          std::string("Could not create directory iterator for \"") +
          dir_name +
          "\". " +
          ec.message());
      }
    }
  }
  if (!filesystem_directory_iterators_.empty()) {
    filesystem_directory_iterators_it_ = filesystem_directory_iterators_.begin();
    filesystem_directory_iterator_ = filesystem_directory_iterators_it_->first;
  } else {
    filesystem_directory_iterators_it_ = filesystem_directory_iterators_.end();
  }
  auto dirs_file = fs::path{dir_name} / "directories.txt";
  if (JNIHelper::GetInstance()->PathExists(dirs_file.c_str(), StorageType::RESOURCES)) {
    std::vector<uint8_t> buffer;
    JNIHelper::GetInstance()->ReadFile(
        dirs_file.c_str(),
        &buffer,
        StorageType::RESOURCES);
    std::istringstream isstr{std::string((char *) buffer.data(), buffer.size())};
    std::string line;
    while (std::getline(isstr, line)) {
      if (!line.empty()) {
        subdirs_.push_back(line);
      }
    }
  }
  subdir_it_ = subdirs_.begin();
}

DirectoryIterator::~DirectoryIterator() = default;

DirectoryIterator& DirectoryIterator::operator ++() {
  if (subdir_iterator_not_at_end()) {
    ++subdir_it_;
  }
  if (!subdir_iterator_not_at_end() &&
      (filesystem_directory_iterator_ != fs::end(filesystem_directory_iterator_)))
  {
    ++filesystem_directory_iterator_;
    if (filesystem_directory_iterator_ == fs::end(filesystem_directory_iterator_)) {
      ++filesystem_directory_iterators_it_;
      if (filesystem_directory_iterators_it_ != filesystem_directory_iterators_.end()) {
        filesystem_directory_iterator_ = filesystem_directory_iterators_it_->first;
      }
    }
  }
  if (!subdir_iterator_not_at_end() &&
      (filesystem_directory_iterator_ == fs::end(filesystem_directory_iterator_)) &&
      (current_asset_filename_ != nullptr))
  {
    current_asset_filename_ = AAssetDir_getNextFileName(asset_dir_.get());
  }
  return *this;
}

bool DirectoryIterator::operator != (const DirectoryIterator& other) const {
  if (asset_dir_ == nullptr) {
    Mlib::verbose_abort("First operator to DirectoryIterator comparison is the end");
  }
  if (other.asset_dir_ != nullptr) {
    Mlib::verbose_abort("Second operator to DirectoryIterator comparison is not the end");
  }
  return (subdir_iterator_not_at_end())
      || (filesystem_directory_iterator_ != fs::end(filesystem_directory_iterator_))
      || (current_asset_filename_ != nullptr);
}

DirectoryEntry DirectoryIterator::operator *() const {
  if (asset_dir_ == nullptr) {
    Mlib::verbose_abort("Derefenciation of end() or a move source");
  }
  if (subdir_iterator_not_at_end()) {
    return {fs::path{dir_name_} / *subdir_it_, true};
  } else if (filesystem_directory_iterator_ != fs::end(filesystem_directory_iterator_)) {
    std::error_code ec;
    bool is_directory = filesystem_directory_iterator_->is_directory(ec);
    if (ec) {
        THROW_OR_ABORT("Could not check if path \"" + filesystem_directory_iterator_->path().string() + "\" is a directory. " + ec.message());
    }
    return {fs::relative(*filesystem_directory_iterator_, JNIHelper::GetInstance()->GetFilesDir(filesystem_directory_iterators_it_->second)), is_directory};
  } else if (current_asset_filename_ != nullptr) {
    return {fs::path{dir_name_} / current_asset_filename_, false};
  } else {
    Mlib::verbose_abort("Derefenciation past the end");
  }
}

bool DirectoryIterator::subdir_iterator_not_at_end() const {
  // https://stackoverflow.com/questions/41384793/does-stdmove-invalidate-iterators
  // After std::move, iterators (other than the end iterator) remain valid.
  // The "begin(...)" function for the DirectoryIterator looks as follows:
  // inline ndk_helper::DirectoryIterator begin(ndk_helper::DirectoryIterator& it) {
  //  return std::move(it);
  //}
  return !subdirs_.empty() && (subdir_it_ != subdirs_.end());
}

std::string JNIHelper::GetFilesDir(StorageType fileStorageType) {
  if (activity_ == nullptr) {
    Mlib::verbose_abort(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
  }

  // Lock mutex
  std::scoped_lock lock{mutex_};

  // Without the cache below, the app crashes when many
  // calls to this function are made during Activity.onCreate.

  // First, try reading from externalFileDir;
  JNIEnv* env = AttachCurrentThread();

  jstring strPath = GetFilesDirJString(env, fileStorageType);
  const char* path = env->GetStringUTFChars(strPath, nullptr);
  if (!path) {
    Mlib::verbose_abort("Could not get external files dir UTF chars");
  }
  std::string s(path);

  env->ReleaseStringUTFChars(strPath, path);
  env->DeleteLocalRef(strPath);
  return s;
}

std::string JNIHelper::ConvertString(const char* str, const char* encode) {
  if (activity_ == nullptr) {
    Mlib::verbose_abort(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
  }

  // Lock mutex
  std::scoped_lock lock{mutex_};

  JNIEnv* env = AttachCurrentThread();
  env->PushLocalFrame(16);

  auto iLength = Mlib::integral_cast<int32_t>(strlen((const char*)str));

  jbyteArray array = env->NewByteArray(iLength);
  env->SetByteArrayRegion(array, 0, iLength, (const signed char*)str);

  jstring strEncode = env->NewStringUTF(encode);

  jclass cls = env->FindClass("java/lang/String");
  jmethodID ctor = env->GetMethodID(cls, "<init>", "([BLjava/lang/String;)V");
  jstring object = (jstring)env->NewObject(cls, ctor, array, strEncode);

  const char* cparam = env->GetStringUTFChars(object, NULL);

  std::string s = std::string(cparam);

  env->ReleaseStringUTFChars(object, cparam);
  env->DeleteLocalRef(array);
  env->DeleteLocalRef(strEncode);
  env->DeleteLocalRef(object);
  env->DeleteLocalRef(cls);

  env->PopLocalFrame(NULL);

  return s;
}
/*
 * Retrieve string resource with a given name
 * arguments:
 *  in: resourceName, name of string resource to retrieve
 * return: string resource value, returns "" when there is no string resource
 * with given name
 */
std::string JNIHelper::GetStringResource(const std::string& resourceName) {
  if (activity_ == NULL) {
    LOGE(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return std::string("");
  }

  // Lock mutex
  std::scoped_lock lock{mutex_};

  JNIEnv* env = AttachCurrentThread();
  jstring name = env->NewStringUTF(resourceName.c_str());

  jstring ret = (jstring)CallObjectMethod(
      "getStringResource", "(Ljava/lang/String;)Ljava/lang/String;", name);

  const char* resource = env->GetStringUTFChars(ret, NULL);
  std::string s = std::string(resource);

  env->ReleaseStringUTFChars(ret, resource);
  env->DeleteLocalRef(ret);
  env->DeleteLocalRef(name);

  return s;
}
/*
 * Audio helpers
 */
int32_t JNIHelper::GetNativeAudioBufferSize() {
  if (activity_ == NULL) {
    LOGE(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return 0;
  }

  JNIEnv* env = AttachCurrentThread();
  jmethodID mid = env->GetMethodID(jni_helper_java_class_,
                                   "getNativeAudioBufferSize", "()I");
  int32_t i = env->CallIntMethod(jni_helper_java_ref_, mid);
  return i;
}

int32_t JNIHelper::GetNativeAudioSampleRate() {
  if (activity_ == NULL) {
    LOGE(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return 0;
  }

  JNIEnv* env = AttachCurrentThread();
  jmethodID mid = env->GetMethodID(jni_helper_java_class_,
                                   "getNativeAudioSampleRate", "()I");
  int32_t i = env->CallIntMethod(jni_helper_java_ref_, mid);
  return i;
}

//---------------------------------------------------------------------------
// Misc implementations
//---------------------------------------------------------------------------
jclass JNIHelper::RetrieveClass(JNIEnv* jni, const char* class_name) {
  jclass activity_class = jni->FindClass(NATIVEACTIVITY_CLASS_NAME);
  jmethodID get_class_loader = jni->GetMethodID(
      activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;");
  jobject cls = jni->CallObjectMethod(activity_->clazz, get_class_loader);
  jclass class_loader = jni->FindClass("java/lang/ClassLoader");
  jmethodID find_class = jni->GetMethodID(
      class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

  jstring str_class_name = jni->NewStringUTF(class_name);
  jclass class_retrieved =
      (jclass)jni->CallObjectMethod(cls, find_class, str_class_name);
  jni->DeleteLocalRef(str_class_name);
  jni->DeleteLocalRef(activity_class);
  jni->DeleteLocalRef(class_loader);
  return class_retrieved;
}

jstring JNIHelper::GetFilesDirJString(JNIEnv* env, StorageType storageType) {
  if (activity_ == nullptr) {
    Mlib::verbose_abort(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
  }

  // Invoking getExternalFilesDir() java API
  jclass cls_Env = env->FindClass(NATIVEACTIVITY_CLASS_NAME);
  jobject obj_File;
  if (storageType == StorageType::EXTERNAL) {
    jmethodID mid = env->GetMethodID(cls_Env, "getExternalFilesDir",
                                     "(Ljava/lang/String;)Ljava/io/File;");
    if (mid == nullptr) {
      Mlib::verbose_abort("Could not get \"getExternalFilesDir\" method ID");
    }
    obj_File = env->CallObjectMethod(activity_->clazz, mid, nullptr);
    if (obj_File == nullptr) {
      Mlib::verbose_abort("Could not call \"getExternalFilesDir\" method");
    }
  } else if (storageType == StorageType::CACHE) {
    jmethodID mid = env->GetMethodID(cls_Env, "getCacheDir",
                                     "()Ljava/io/File;");
    if (mid == nullptr) {
      Mlib::verbose_abort("Could not get \"getCacheDir\" method ID");
    }
    obj_File = env->CallObjectMethod(activity_->clazz, mid);
    if (obj_File == nullptr) {
      Mlib::verbose_abort("Could not call \"getCacheDir\" method");
    }
  } else {
    Mlib::verbose_abort("Unknown file storage type: " + std::to_string((int)storageType));
  }
  jclass cls_File = env->FindClass("java/io/File");
  jmethodID mid_getPath =
      env->GetMethodID(cls_File, "getPath", "()Ljava/lang/String;");
  auto obj_Path = (jstring)env->CallObjectMethod(obj_File, mid_getPath);
  if (obj_Path == nullptr) {
    Mlib::verbose_abort("getPath returned null");
  }
  return obj_Path;
}

struct JniThreadLocal {
    JniThreadLocal& operator = (const JniThreadLocal&) = delete;
    explicit JniThreadLocal(nullptr_t)
        : activity{ nullptr }
        , env{ nullptr }
    {}
    JniThreadLocal(const JniThreadLocal& other)
        : JniThreadLocal{ nullptr }
    {
        if (other.activity != nullptr) {
            THROW_OR_ABORT("JniThreadLocal already has an activity");
        }
        if (other.env != nullptr) {
            THROW_OR_ABORT("JniThreadLocal already has an environment");
        }
    }
    ~JniThreadLocal() {
        DetachCurrentThreadIfNecessary();
    }
    void DetachCurrentThreadIfNecessary() {
        if (activity != nullptr) {
            // Unregister this thread from the VM
            // https://stackoverflow.com/a/59935021/2292832:
            //   The best solution is to only attach once to
            //   the thread and let it running its course
            //   and automatically detach with thread local storage
            //   (C++ 11 or higher) when the thread exits.
            activity->vm->DetachCurrentThread();
            activity = nullptr;
            env = nullptr;
        }
    }
    ANativeActivity *activity;
    JNIEnv *env;
};

static JniThreadLocal& GetJniThreadLocal() {
    static THREAD_LOCAL(JniThreadLocal) jniTlsManager =
        JniThreadLocal{ nullptr };
    return jniTlsManager;
}

JNIEnv* JNIHelper::AttachCurrentThread() {
    {
        JNIEnv *env;
        if (activity_->vm->GetEnv((void **) &env, JNI_VERSION_1_4) == JNI_OK)
            return env;
    }
    JniThreadLocal& jniTls = GetJniThreadLocal();
    if (jniTls.env == nullptr) {
        if (activity_->vm->AttachCurrentThread(&jniTls.env, nullptr) != JNI_OK) {
          Mlib::verbose_abort("Could not attach current thread");
        }
        if (jniTls.env == nullptr) {
          Mlib::verbose_abort("Env is null after attach current thread");
        }
        jniTls.activity = activity_;
    }
    return jniTls.env;
}

void JNIHelper::DetachCurrentThreadIfNecessary() {
    JniThreadLocal& jniTls = GetJniThreadLocal();
    jniTls.DetachCurrentThreadIfNecessary();
}

void JNIHelper::DeleteObject(jobject obj) {
  if (obj == nullptr) {
    Mlib::verbose_abort("obj can not be NULL");
  }

  JNIEnv* env = AttachCurrentThread();
  env->DeleteGlobalRef(obj);
}

jobject JNIHelper::CallObjectMethod(const char* strMethodName,
                                    const char* strSignature, ...) {
  if (activity_ == NULL) {
    LOGE(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return NULL;
  }

  JNIEnv* env = AttachCurrentThread();
  jmethodID mid =
      env->GetMethodID(jni_helper_java_class_, strMethodName, strSignature);
  if (mid == NULL) {
    LOGE("method ID %s, '%s' not found", strMethodName, strSignature);
    return NULL;
  }

  va_list args;
  va_start(args, strSignature);
  jobject obj = env->CallObjectMethodV(jni_helper_java_ref_, mid, args);
  va_end(args);

  return obj;
}

void JNIHelper::CallVoidMethod(const char* strMethodName,
                               const char* strSignature, ...) {
  if (activity_ == NULL) {
    LOGE(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return;
  }

  JNIEnv* env = AttachCurrentThread();
  jmethodID mid =
      env->GetMethodID(jni_helper_java_class_, strMethodName, strSignature);
  if (mid == NULL) {
    LOGE("method ID %s, '%s' not found", strMethodName, strSignature);
    return;
  }
  va_list args;
  va_start(args, strSignature);
  env->CallVoidMethodV(jni_helper_java_ref_, mid, args);
  va_end(args);

  return;
}

jobject JNIHelper::CallObjectMethod(jobject object, const char* strMethodName,
                                    const char* strSignature, ...) {
  if (activity_ == NULL) {
    LOGE(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return NULL;
  }

  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->GetObjectClass(object);
  jmethodID mid = env->GetMethodID(cls, strMethodName, strSignature);
  if (mid == NULL) {
    LOGE("method ID %s, '%s' not found", strMethodName, strSignature);
    return NULL;
  }

  va_list args;
  va_start(args, strSignature);
  jobject obj = env->CallObjectMethodV(object, mid, args);
  va_end(args);

  env->DeleteLocalRef(cls);
  return obj;
}

void JNIHelper::CallVoidMethod(jobject object, const char* strMethodName,
                               const char* strSignature, ...) {
  if (activity_ == NULL) {
    LOGE(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return;
  }

  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->GetObjectClass(object);
  jmethodID mid = env->GetMethodID(cls, strMethodName, strSignature);
  if (mid == NULL) {
    LOGE("method ID %s, '%s' not found", strMethodName, strSignature);
    return;
  }

  va_list args;
  va_start(args, strSignature);
  env->CallVoidMethodV(object, mid, args);
  va_end(args);

  env->DeleteLocalRef(cls);
  return;
}

float JNIHelper::CallFloatMethod(jobject object, const char* strMethodName,
                                 const char* strSignature, ...) {
  float f = 0.f;
  if (activity_ == NULL) {
    LOGE(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return f;
  }

  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->GetObjectClass(object);
  jmethodID mid = env->GetMethodID(cls, strMethodName, strSignature);
  if (mid == NULL) {
    LOGE("method ID %s, '%s' not found", strMethodName, strSignature);
    return f;
  }
  va_list args;
  va_start(args, strSignature);
  f = env->CallFloatMethodV(object, mid, args);
  va_end(args);

  env->DeleteLocalRef(cls);
  return f;
}

int32_t JNIHelper::CallIntMethod(jobject object, const char* strMethodName,
                                 const char* strSignature, ...) {
  int32_t i = 0;
  if (activity_ == NULL) {
    LOGE(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return i;
  }

  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->GetObjectClass(object);
  jmethodID mid = env->GetMethodID(cls, strMethodName, strSignature);
  if (mid == NULL) {
    LOGE("method ID %s, '%s' not found", strMethodName, strSignature);
    return i;
  }
  va_list args;
  va_start(args, strSignature);
  i = env->CallIntMethodV(object, mid, args);
  va_end(args);

  env->DeleteLocalRef(cls);
  return i;
}

bool JNIHelper::CallBooleanMethod(jobject object, const char* strMethodName,
                                  const char* strSignature, ...) {
  bool b;
  if (activity_ == NULL) {
    LOGE(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return false;
  }

  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->GetObjectClass(object);
  jmethodID mid = env->GetMethodID(cls, strMethodName, strSignature);
  if (mid == NULL) {
    LOGE("method ID %s, '%s' not found", strMethodName, strSignature);
    return false;
  }
  va_list args;
  va_start(args, strSignature);
  b = env->CallBooleanMethodV(object, mid, args);
  va_end(args);

  env->DeleteLocalRef(cls);
  return b;
}

jobject JNIHelper::CreateObject(const char* class_name) {
  JNIEnv* env = AttachCurrentThread();

  jclass cls = env->FindClass(class_name);
  jmethodID constructor = env->GetMethodID(cls, "<init>", "()V");

  jobject obj = env->NewObject(cls, constructor);
  jobject objGlobal = env->NewGlobalRef(obj);
  env->DeleteLocalRef(obj);
  env->DeleteLocalRef(cls);
  return objGlobal;
}

void JNIHelper::RunOnUiThread(std::function<void()> callback) {
  // Lock mutex
  std::scoped_lock lock{mutex_};

  JNIEnv* env = AttachCurrentThread();
  static jmethodID mid = NULL;
  if (mid == NULL)
    mid = env->GetMethodID(jni_helper_java_class_, "runOnUIThread", "(J)V");

  // Allocate temporary function object to be passed around
  std::function<void()>* pCallback = new std::function<void()>(callback);
  env->CallVoidMethod(jni_helper_java_ref_, mid, (int64_t)pCallback);
}

// This JNI function is invoked from UIThread asynchronously
extern "C" {
JNIEXPORT void Java_com_sample_helper_NDKHelper_RunOnUiThreadHandler(
    JNIEnv* env, jobject thiz, int64_t pointer) {
  std::function<void()>* pCallback = (std::function<void()>*)pointer;
  (*pCallback)();

  // Deleting temporary object
  delete pCallback;
}
}

}  // namespace ndkHelper
