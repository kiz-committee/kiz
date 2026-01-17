#pragma once
#include "bigint.hpp"
#include <vector>
#include <memory>
#include <sstream>
#include <utility>
#include <functional>

namespace dep {

template <typename VT>
class Dict {
public:
    // 桶节点：键为 BigInt，存储键值对与链表指针
    struct Bucket {
        BigInt key;          // 键类型改为 BigInt（支持哈希后的值）
        VT value;            // 值类型保持泛型
        std::shared_ptr<Bucket> next;

        // 构造函数：直接接收 BigInt 键，无需计算字符串哈希
        Bucket(const BigInt& k, VT val)
            : key(k), value(std::move(val)), next(nullptr) {}

        Bucket(BigInt&& k, VT&& val)
            : key(std::move(k)), value(std::move(val)), next(nullptr) {}
    };

    using Node = Bucket;
    std::vector<std::shared_ptr<Node>> buckets_;  // 桶数组（链表头指针）
    size_t elem_count_ = 0;                       // 元素总数
    const float load_factor_ = 0.7f;              // 负载因子（const 不可修改）

    /**
     * @brief 从 BigInt 键生成桶索引（基于 BigInt 的字符串哈希）
     * 保证相同 BigInt 键始终映射到相同索引
     */
    [[nodiscard]] size_t getBucketIndex(const BigInt& key) const {
        std::string key_str = key.to_string();
        size_t hash = std::hash<std::string>()(key_str);
        return hash & (buckets_.size() - 1);  // 位运算取模，要求桶大小为 2 的幂
    }

    /**
     * @brief 扩容：桶大小翻倍，重新哈希所有元素
     * 修复原逻辑中索引计算依赖节点 hash 的耦合问题
     */
    void resize() {
        const size_t old_size = buckets_.size();
        const size_t new_size = old_size * 2;
        std::vector<std::shared_ptr<Node>> new_buckets(new_size, nullptr);

        for (size_t i = 0; i < old_size; ++i) {
            std::shared_ptr<Node> current = buckets_[i];
            while (current != nullptr) {
                std::shared_ptr<Node> next_node = current->next;
                // 基于当前节点的 BigInt 键重新计算新索引
                size_t new_idx = getBucketIndex(current->key);
                // 头插法插入新桶
                current->next = new_buckets[new_idx];
                new_buckets[new_idx] = current;
                current = next_node;
            }
        }

        buckets_.swap(new_buckets);
    }

    // ========================= 构造与析构 =========================
    explicit Dict() {
        constexpr size_t init_size = 16;  // 初始桶大小为 2 的幂
        buckets_.resize(init_size, nullptr);
    }

    // 用 BigInt 键值对 vector 初始化
    explicit Dict(const std::vector<std::pair<BigInt, VT>>& vec) {
        // 计算初始桶大小，保证负载因子不超限
        size_t init_size = 16;
        while (init_size < (vec.size() / load_factor_)) {
            init_size *= 2;
        }
        buckets_.resize(init_size, nullptr);

        for (const auto& [key, val] : vec) {
            insert(key, val);
        }
    }

    ~Dict() = default;  // shared_ptr 自动管理内存

    // ========================= 拷贝与移动语义 =========================
    // 深拷贝构造：逐节点拷贝键值对
    Dict(const Dict& other)
        : load_factor_(other.load_factor_) {  // const 成员只能初始化，不能赋值
        auto all_kv = other.to_vector();
        size_t init_size = 16;
        while (init_size < (all_kv.size() / load_factor_)) {
            init_size *= 2;
        }
        buckets_.resize(init_size, nullptr);

        for (const auto& [key, val] : all_kv) {
            insert(key, val);
        }
        elem_count_ = other.elem_count_;
    }

    // 移动构造：转移资源所有权
    Dict(Dict&& other) noexcept
        : buckets_(std::move(other.buckets_)),
          elem_count_(other.elem_count_),
          load_factor_(other.load_factor_) {  // const 成员直接拷贝初始化
        other.elem_count_ = 0;
        other.buckets_.clear();
    }

    // 深拷贝赋值
    Dict& operator=(const Dict& other) {
        if (this == &other) return *this;

        // 清空当前资源
        buckets_.clear();
        elem_count_ = 0;

        // 重新初始化桶并插入数据
        auto all_kv = other.to_vector();
        size_t init_size = 16;
        while (init_size < (all_kv.size() / load_factor_)) {
            init_size *= 2;
        }
        buckets_.resize(init_size, nullptr);

        for (const auto& [key, val] : all_kv) {
            insert(key, val);
        }
        elem_count_ = all_kv.size();

        return *this;
    }

    // 移动赋值：修复原逻辑中修改 const 成员的 Bug
    Dict& operator=(Dict&& other) noexcept {
        if (this == &other) return *this;

        // 清空当前资源
        buckets_.clear();
        elem_count_ = 0;

        // 转移 other 的资源（不修改 load_factor_）
        buckets_ = std::move(other.buckets_);
        elem_count_ = other.elem_count_;

        // 置空原对象
        other.elem_count_ = 0;
        other.buckets_.clear();

        return *this;
    }

    // ========================= 核心操作：插入与查找 =========================
    /**
     * @brief 插入/更新键值对（基于 BigInt 键）
     * @param key BigInt 类型键（支持已哈希值）
     * @param val 要存储的值
     * @return 旧值（若存在），否则返回 VT()
     */
    VT insert(const BigInt& key, VT val) {
        if (buckets_.empty()) {
            buckets_.resize(16, nullptr);
        }

        size_t bucket_idx = getBucketIndex(key);
        VT old_val = VT();

        // 检查键是否已存在：存在则更新值
        std::shared_ptr<Node> current = buckets_[bucket_idx];
        while (current != nullptr) {
            if (current->key == key) {
                old_val = std::move(current->value);
                current->value = std::move(val);
                return old_val;
            }
            current = current->next;
        }

        // 检查负载因子，触发扩容
        if (static_cast<float>(elem_count_) / buckets_.size() >= load_factor_) {
            resize();
            // 扩容后重新计算索引
            bucket_idx = getBucketIndex(key);
        }

        // 插入新节点（头插法）
        auto new_node = std::make_shared<Node>(key, std::move(val));
        new_node->next = buckets_[bucket_idx];
        buckets_[bucket_idx] = new_node;
        elem_count_++;

        return old_val;
    }

    /**
     * @brief 查找 BigInt 键对应的节点
     * @return 找到返回节点指针，否则返回 nullptr
     */
    [[nodiscard]] std::shared_ptr<Node> find(const BigInt& key) const {
        if (buckets_.empty()) {
            return nullptr;
        }

        size_t bucket_idx = getBucketIndex(key);
        std::shared_ptr<Node> current = buckets_[bucket_idx];

        while (current != nullptr) {
            if (current->key == key) {
                return current;
            }
            current = current->next;
        }

        return nullptr;
    }

    // ========================= 辅助方法 =========================
    /**
     * @brief 转换为字符串（支持 VT 类型的 to_string 方法或指针）
     */
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << "{ ";

        size_t printed = 0;
        for (const auto& bucket_head : buckets_) {
            auto current = bucket_head;
            while (current != nullptr) {
                ss << current->key.to_string() << ": ";
                if constexpr (std::is_pointer_v<VT>) {
                    ss << static_cast<void*>(current->value);
                } else {
                    ss << current->value.to_string();
                }

                printed++;
                if (printed < elem_count_) {
                    ss << ", ";
                }
                current = current->next;
            }
        }

        ss << " }";
        return ss.str();
    }

    /**
     * @brief 转换为 BigInt 键值对 vector
     */
    [[nodiscard]] std::vector<std::pair<BigInt, VT>> to_vector() const {
        std::vector<std::pair<BigInt, VT>> vec;
        for (const auto& bucket_head : buckets_) {
            auto current = bucket_head;
            while (current != nullptr) {
                vec.emplace_back(current->key, current->value);
                current = current->next;
            }
        }
        return vec;
    }

    /**
     * @brief 获取元素数量
     */
    [[nodiscard]] size_t size() const {
        return elem_count_;
    }

    /**
     * @brief 判断是否为空
     */
    [[nodiscard]] bool empty() const {
        return elem_count_ == 0;
    }
};

} // namespace dep