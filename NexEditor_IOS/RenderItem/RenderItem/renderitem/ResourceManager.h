#pragma once

#include <map>

class ManagerInterface{

public:
	ManagerInterface(){};
	virtual ~ManagerInterface(){};
	virtual void gc() = 0;
	virtual int forceGC() = 0;
};

template<typename T>
class ResourceManager :public ManagerInterface{

	typedef std::map<std::string, T*> mapper_t;

	mapper_t mapper_;

public:
	ResourceManager(){

	}

	~ResourceManager(){

		for (typename mapper_t::iterator itor = mapper_.begin(); itor != mapper_.end(); ++itor){

			T* res = itor->second;
			delete res;
		}
	}

	T* get(const std::string& key){

		typename mapper_t::iterator itor = mapper_.find(key);
		if (itor != mapper_.end()){

			T* res = itor->second;
			return res;
		}

		return NULL;
	}

	bool insert(const std::string& key, T* res){

		if (get(key)){

			return false;
		}
		mapper_.insert(make_pair(key, res));
		return true;
	}

	void gc(){

		for (typename mapper_t::iterator itor = mapper_.begin(); itor != mapper_.end();){

			T* res = itor->second;
			if (res->getRef() == 0){

				if (res->decLife() <= 0){

					delete res;
					typename mapper_t::iterator itor_current = itor;
					typename mapper_t::iterator itor_next = ++itor;
					mapper_.erase(itor_current);
					itor = itor_next;
				}
				else
					++itor;
			}
			else
				++itor;
		}
	}

	int forceGC(){

		for (typename mapper_t::iterator itor = mapper_.begin(); itor != mapper_.end();){

			T* res = itor->second;
			if (res->getRef() == 0){

				delete res;
				typename mapper_t::iterator itor_current = itor;
				typename mapper_t::iterator itor_next = ++itor;
				mapper_.erase(itor_current);
				itor = itor_next;
			}
			else
				++itor;
		}

		return (mapper_.size() <= 0)?1:0;
	}
};