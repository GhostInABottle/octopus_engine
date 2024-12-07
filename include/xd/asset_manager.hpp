#ifndef H_XD_ASSET_MANAGER
#define H_XD_ASSET_MANAGER
#include <any>
#include <unordered_map>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace xd
{
    class asset_manager
    {
    public:
        template <typename T, typename... Args>
        bool contains_key(const std::string& cache_key) {
            auto& assets = get_asset_map<T>();
            return assets.find(cache_key) != assets.end();
        }

        template <typename T, typename... Args>
        std::shared_ptr<T> get(const std::string& cache_key) {
            auto& assets = get_asset_map<T>();
            auto asset = assets.find(cache_key);
            if (asset == assets.end()) {
                throw std::out_of_range { "Trying to load non-cached asset " + cache_key };
            }

            return asset->second;
        }

        template <typename T, typename... Args>
        std::shared_ptr<T> load(const std::string& cache_key, Args&&... args) {
            // check if it's in the map
            auto& assets = get_asset_map<T>();
            auto it = assets.find(cache_key);
            if (it != assets.end()) {
                return it->second;
            }

            // not loaded, create it
            auto resource = std::make_shared<T>(std::forward<Args>(args)...);
            assets.emplace(cache_key, resource);
            return resource;
        }

        template <typename T>
        void release(const std::string& cache_key)
        {
            auto& assets = get_asset_map<T>();
            assets.erase(cache_key);
        }

    private:
        std::unordered_map<std::size_t, std::any> m_asset_type_map;

        template <typename T>
        std::unordered_map<std::string, std::shared_ptr<T>>& get_asset_map()
        {
            typedef std::unordered_map<std::string, std::shared_ptr<T>> asset_map_type;
            std::size_t hash = typeid(T).hash_code();
            auto it = m_asset_type_map.find(hash);
            if (it == m_asset_type_map.end()) {
                m_asset_type_map.emplace(hash, asset_map_type());
            }

            return *std::any_cast<asset_map_type>(&m_asset_type_map[hash]);
        }
    };
}

#endif
