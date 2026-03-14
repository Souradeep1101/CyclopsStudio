#pragma once

#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <thread>
#include <iostream>

namespace Cyclops
{
	/**
	 * @struct ProfileResult
	 * @brief Data packet representing a single profiled function call.
	 */
	struct ProfileResult
	{
		std::string Name;
		long long Start, End;
		uint32_t ThreadID;
	};

	struct InstrumentationSession
	{
		std::string Name;
	};

	/**
	 * @class Instrumentor
	 * @brief The Singleton Profiler. Writes execution trace data to a JSON file.
	 * @details The output format is compatible with 'chrome://tracing' and 'Perfetto'.
	 */
	class Instrumentor
	{
	private:
		InstrumentationSession* m_CurrentSession;
		std::ofstream m_OutputStream;
		int m_ProfileCount;
	public:
		Instrumentor()
			: m_CurrentSession(nullptr), m_ProfileCount(0)
		{
		}

		/// @brief Access the Singleton instance.
		static Instrumentor& Get()
		{
			static Instrumentor instance;
			return instance;
		}

		void BeginSession(const std::string& name, const std::string& filepath = "results.json")
		{
			m_OutputStream.open(filepath);
			WriteHeader();
			m_CurrentSession = new InstrumentationSession{ name };
		}

		void EndSession()
		{
			WriteFooter();
			m_OutputStream.close();
			delete m_CurrentSession;
			m_CurrentSession = nullptr;
			m_ProfileCount = 0;
		}

		void WriteProfile(const ProfileResult& result)
		{
			if (m_ProfileCount++ > 0)
				m_OutputStream << ",";

			std::string name = result.Name;
			std::replace(name.begin(), name.end(), '"', '\'');

			m_OutputStream << "{";
			m_OutputStream << "\"cat\":\"function\",";
			m_OutputStream << "\"dur\":" << (result.End - result.Start) << ",";
			m_OutputStream << "\"name\":\"" << name << "\",";
			m_OutputStream << "\"ph\":\"X\",";
			m_OutputStream << "\"pid\":0,";
			m_OutputStream << "\"tid\":" << result.ThreadID << ",";
			m_OutputStream << "\"ts\":" << result.Start;
			m_OutputStream << "}";

			m_OutputStream.flush();
		}

		void WriteHeader()
		{
			m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
			m_OutputStream.flush();
		}

		void WriteFooter()
		{
			m_OutputStream << "]}";
			m_OutputStream.flush();
		}
	};

	/**
	 * @class InstrumentationTimer
	 * @brief RAII Timer. Starts clock on construction, writes profile on destruction.
	 */
	class InstrumentationTimer
	{
	public:
		InstrumentationTimer(const char* name)
			: m_Name(name), m_Stopped(false)
		{
			m_StartTimepoint = std::chrono::high_resolution_clock::now();
		}

		~InstrumentationTimer()
		{
			if (!m_Stopped)
				Stop();
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::high_resolution_clock::now();

			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
			long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			uint32_t threadID = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
			Instrumentor::Get().WriteProfile({ m_Name, start, end, threadID });

			m_Stopped = true;
		}
	private:
		const char* m_Name;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
		bool m_Stopped;
	};
}

// =========================================================================================
//                                   INSTRUMENTATION MACROS
// =========================================================================================
// Use these macros to profile your code. They are designed to be easily stripped out
// in Release builds if necessary by redefining them to empty.

/**
 * @def CYCLOPS_PROFILE_BEGIN_SESSION(name, filepath)
 * @brief Starts a new profiling session.
 * @param name Name of the session (e.g., "Runtime", "Startup").
 * @param filepath Path where the JSON result file will be saved (e.g., "results.json").
 * @note Call this once at the start of the application or phase you want to measure.
 */
#define CYCLOPS_PROFILE_BEGIN_SESSION(name, filepath) ::Cyclops::Instrumentor::Get().BeginSession(name, filepath)

 /**
  * @def CYCLOPS_PROFILE_END_SESSION()
  * @brief Ends the current profiling session.
  * @note Flushes data to disk and closes the file stream. Call this before shutdown.
  */
#define CYCLOPS_PROFILE_END_SESSION() ::Cyclops::Instrumentor::Get().EndSession()

  /**
   * @def CYCLOPS_PROFILE_SCOPE(name)
   * @brief Profiles a specific scope by name.
   * @param name The custom string to display in the Chrome Tracing timeline.
   * @details Creates a timer on the stack that stops when the current scope {} ends.
   * Example: { CYCLOPS_PROFILE_SCOPE("Physics Calc"); DoPhysics(); }
   */
#define CYCLOPS_PROFILE_SCOPE(name) ::Cyclops::InstrumentationTimer timer##__LINE__(name)

   /**
	* @def CYCLOPS_PROFILE_FUNCTION()
	* @brief Automatically profiles the current function using its signature as the name.
	* @details Uses compiler intrinsics (__FUNCSIG__ on MSVC) to get the full function name.
	* Place this at the very top of any function you want to measure.
	*/
#define CYCLOPS_PROFILE_FUNCTION() CYCLOPS_PROFILE_SCOPE(__FUNCSIG__)