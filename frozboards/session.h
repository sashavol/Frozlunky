#pragma once

#include <string>
#include <functional>
#include <memory>

#include "score_info.h"

#define SESSION_SUCCESS 1
#define SESSION_ERR_FAILED_REQ 2
#define SESSION_ERR_ALREADY_PLAYED 4
#define SESSION_VERIFYING_USER 8

#define SESSION_MAX_ATTEMPTS 10

namespace Frozboards {
	class Session {
	public:
		typedef std::function<void(std::shared_ptr<Session>, int code)> session_callback;
		typedef std::function<void(const std::string&)> string_callback;
		typedef std::function<void()> invalidate_callback;

	private:
		std::string session_token;
		std::string auth_token;
		std::string seed;
		std::string sid;

		//validity determined during initialization and score submission
		bool valid;
		invalidate_callback icb;
		double best_score;
		int runs_submitted;

	public:
		Session(const std::string& sid, 
			const std::string& se_token, 
			const std::string& au_token, 
			const std::string& seed) : session_token(se_token), auth_token(au_token), seed(seed), valid(true), best_score(0.0), runs_submitted(0) {}

		void submit(TrackedValuesSet& tvs, bool success, string_callback url_callback=string_callback());

		void set_invalidate_callback(invalidate_callback cb) {
			this->icb = cb;
		}

		double best() {
			return best_score;
		}

		int run_count() {
			return runs_submitted;
		}

		void process_score(double score) {
			if(score > best_score)
				best_score = score;
		}

		bool invalidated() {
			return !valid;
		}

		std::string session_seed() {
			return seed;
		}

	private:
		typedef std::function<void(bool)> equ_init_callback;
		static void InitializeEquation(equ_init_callback result);

	public:
		static double ComputePointScore(const Frozboards::TrackedValues& tv);
		static void CreateSession(const std::string& sid, session_callback cb);
	};
}