// Auxid: The Orthodox C++ Platform.
// Copyright (C) 2026 IAS (ias@iasoft.dev)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <platform/platform.hpp>

#include <vendor/sheredom/subprocess.h>

#include <curl/curl.h>

namespace au::platform
{
  auto download_file(String url, String dst_path) -> Result<void>
  {
    const auto write_chunk = [](void *ptr, usize size, usize nmemb, FILE *stream) -> usize {
      return fwrite(ptr, size, nmemb, stream);
    };

    const auto dst_file = fopen(dst_path.c_str(), "wb");
    if (!dst_file)
      return fail("failed to open file '%s' for writing", dst_path.c_str());

    curl_global_init(CURL_GLOBAL_DEFAULT);
    const auto curl = curl_easy_init();

    if (!curl)
    {
      fclose(dst_file);
      curl_global_cleanup();
      return fail("failed to initialize curl");
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, dst_file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    const auto res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      fclose(dst_file);
      curl_easy_cleanup(curl);
      curl_global_cleanup();
      return fail("failed to download file with error '%s'", curl_easy_strerror(res));
    }

    fclose(dst_file);
    curl_easy_cleanup(curl);

    curl_global_cleanup();

    return {};
  }

  auto spawn_process(std::initializer_list<const char *> command_line) -> Result<Pair<i32, String>>
  {
    Vec<const char *> cl;
    cl.reserve(command_line.size() + 1);
    for (auto t : command_line)
      cl.push_back(t);
    cl.push_back(nullptr);

    struct subprocess_s subprocess;

    i32 options = subprocess_option_combined_stdout_stderr | subprocess_option_search_user_path;

    i32 result = subprocess_create(cl.data(), options, &subprocess);
    if (result != 0)
      return fail("Error: Failed to spawn the process");

    String captured_output;

    captured_output.reserve(8192);

    FILE *p_stdout = subprocess_stdout(&subprocess);
    if (p_stdout)
    {
      char buffer[4096];
      size_t bytes_read;

      while ((bytes_read = fread(buffer, 1, sizeof(buffer), p_stdout)) > 0)
      {
        captured_output.append(StringView(buffer, bytes_read));
      }
    }

    i32 return_code = 0;
    result = subprocess_join(&subprocess, &return_code);
    if (result != 0)
      return fail("Error: Failed to join the process");

    subprocess_destroy(&subprocess);

    return Pair<i32, String>{return_code, captured_output};
  }
} // namespace au::platform