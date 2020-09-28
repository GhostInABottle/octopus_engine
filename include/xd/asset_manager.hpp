#ifndef H_XD_ASSET_MANAGER
#define H_XD_ASSET_MANAGER

#include "asset_serializer.hpp"
#include <any>
#include <unordered_map>
#include <cstddef>
#include <memory>
#include <utility>

namespace xd
{
    class asset_manager
    {
    public:
        template <typename T, typename... Args>
        typename std::shared_ptr<T> load(Args&&... args)
        {
            // create serializer for asset type and get cache key
            asset_serializer<T> serializer;
            auto cache_key = serializer(std::forward<Args>(args)...);
            // check if it's in the persistent map
            auto& persistent_assets = get_persistent_asset_map<T>();
            auto it = persistent_assets.find(cache_key);
            if (it != persistent_assets.end())
                return it->second;
            // otherwise check if it exists in the non-persistent map
            auto& loaded_assets = get_asset_map<T>();
            auto it2 = loaded_assets.find(cache_key);
            if (it2 != loaded_assets.end()) {
                // make sure it has not expired
                if (auto handle = it2->second.lock())
                    return handle;
                // it has expired, remove it
                loaded_assets.erase(it2);
            }
            // not loaded in either map, create it
            auto resource = std::make_shared<T>(std::forward<Args>(args)...);
            loaded_assets.insert(std::make_pair(cache_key, resource));
            return resource;
        }

        template <typename T, typename... Args>
        typename std::shared_ptr<T> load_persistent(Args&&... args)
        {
            // create serializer for asset type and get cache key
            asset_serializer<T> serializer;
            auto cache_key = serializer(std::forward<Args>(args)...);
            // check if it's in the persistent map
            auto& persistent_assets = get_persistent_asset_map<T>();
            auto it = persistent_assets.find(cache_key);
            if (it != persistent_assets.end())
                return it->second;
            // not loaded in either map, create it
            auto resource = std::make_shared<T>(std::forward<Args>(args)...);
            persistent_assets.insert(std::make_pair(cache_key, resource));
            return resource;
        }

        template <typename T>
        void release(std::shared_ptr<T> resource)
        {
            // get the persistent map
            auto& persistent_assets = get_persistent_asset_map<T>();
            // iterate through each loaded asset
            for (auto it = persistent_assets.begin(); it != persistent_assets.end(); ++it) {
                // check whether to remove this resource
                if (resource.get() == it->second.get()) {
                    persistent_assets.erase(it);
                    break;
                }
            }
        }

    private:
        std::unordered_map<std::size_t, std::any> m_asset_type_map;
        std::unordered_map<std::size_t, std::any> m_persistent_asset_type_map;

        template <typename T>
        std::unordered_map<typename asset_serializer<T>::key_type, std::weak_ptr<T>>& get_asset_map()
        {
            typedef std::unordered_map<typename asset_serializer<T>::key_type, std::weak_ptr<T>> asset_map_type;
            std::size_t hash = typeid(T).hash_code();
            auto it = m_asset_type_map.find(hash);
            if (it == m_asset_type_map.end())
                m_asset_type_map.insert(std::make_pair(hash, asset_map_type()));
            return *std::any_cast<asset_map_type>(&m_asset_type_map[hash]);
        }

        template <typename T>
        std::unordered_map<typename asset_serializer<T>::key_type, std::shared_ptr<T>>& get_persistent_asset_map()
        {
            typedef std::unordered_map<typename asset_serializer<T>::key_type, std::shared_ptr<T>> asset_map_type;
            std::size_t hash = typeid(T).hash_code();
            auto it = m_persistent_asset_type_map.find(hash);
            if (it == m_persistent_asset_type_map.end())
                m_persistent_asset_type_map.insert(std::make_pair(hash, asset_map_type()));
            return *std::any_cast<asset_map_type>(&m_persistent_asset_type_map[hash]);
        }
    };
}

#endif
