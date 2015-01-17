#include <stdexcept>
#include <Windows.h>
#include "tile_util.h"
#include <boost/algorithm/string.hpp>
#include <map>

namespace TileUtil {
	std::string QueryTileFile(bool save) {
		char szFile[MAX_PATH];
		szFile[0] = '\0';

		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
	
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "XML Chunk Format (*.xml *.txt)\0*.xml;*.txt\0";
		
		ofn.Flags  = OFN_PATHMUSTEXIST;
		if(!save) {
			ofn.Flags |= OFN_FILEMUSTEXIST;
		}

		BOOL res;
		if(save)
			res = GetSaveFileName(&ofn);
		else
			res = GetOpenFileName(&ofn);

		if(!res) {
			throw std::runtime_error("File open failed.");
		}

		std::string ret(szFile);
		if(ret.find(".xml") != ret.size() - 4 && ret.find(".txt") != ret.size() - 4) {
			ret += ".xml";
		}

		return ret;
	}

	std::string GetBaseFilename(const std::string& file) {
		auto las = file.find_last_of("\\/");
		if(las != std::string::npos) {
			return file.substr(las+1);
		}
		else {
			return file;
		}
	}

	static double score_max_consec(const std::string* s1p, const std::string* s2p) {
		if(s2p->size() < s1p->size())
			std::swap(s1p, s2p);

		const std::string& s1 = *s1p;
		const std::string& s2 = *s2p;
		
		int max_cons = 0;

		for(int p2 = 0, m2 = s2.size(); p2 < m2; ++p2) {
			int cons = 0;
			for(int p1 = 0, m1 = s1.size(); p1 < m1 && p1+p2 < m2; ++p1) {
				if(s1[p1] == s2[p2+p1]) {
					cons++;
				}
			}	
			max_cons = max(max_cons, cons);
		}

		return max_cons / (double)s1.size();
	}
	
	static double score_charcomp(const std::string& s1, const std::string& s2) {
		int same = 0;
		for(char c1 : s1) {
			for(char c2 : s2) {
				if(c1 == c2)
					same++;
			}
		}
		return same / (double)(s1.size()*s2.size());
	}

	static double score_base_equality(const std::string* s1, const std::string* s2) {
		if(s1->size() > s2->size())
			std::swap(s1, s2);

		const std::string& a = *s1;
		const std::string& b = *s2;

		int count = 0;
		for(int p1 = 0, m1 = a.size(); p1 < m1; ++p1) {
			if(a[p1] == b[p1])
				count++;
		}

		return double(count) / double(a.size());
	}

	static void string_normalize(std::string& str) {
		using namespace boost::algorithm;

		trim(str);
		to_upper(str);
		replace_all(str, " ", "");
	}

	std::vector<std::string> Search(const std::vector<std::string>& list, const std::string& query) {
		if(query.empty())
			return std::vector<std::string>();

		std::string comp_query = query;
		string_normalize(comp_query);

		std::map<std::string, double> scores;
		std::vector<std::string> out;
		
		for(const std::string& elem : list) {
			std::string comp = elem;
			string_normalize(comp);
			
			if(comp == comp_query)
				scores[elem] = 10000;
			else
				scores[elem] = (score_base_equality(&comp, &comp_query) + score_max_consec(&comp, &comp_query) + score_charcomp(comp, comp_query));
			
			out.push_back(elem);
		}

		std::sort(out.begin(), out.end(), [&](const std::string& a, const std::string& b) {
			return scores[a] > scores[b];
		});

		return out;
	}
}