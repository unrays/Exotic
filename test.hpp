template<typename T>
class Sparse {
private:
    static constexpr std::size_t DEFAULT_DENSE_CAPACITY = 2048;
    static constexpr std::size_t DEFAULT_SPARSE_CAPACITY = 16384;

    /**
     * @brief Prints a formatted error for insufficient capacity.
     * @param context Context in which the error occurred.
     * @param required The required size.
     * @param actual The actual size.
     */
    inline void error_not_enough_capacity(const std::string& context, size_t required, size_t actual) {
        std::cerr << "[ERROR] " << context
            << " | Required size: " << required
            << ", Actual size: " << actual << std::endl;
    }

    /**
     * @brief Prints a formatted error if an entity is missing a required component.
     * @param entity_id ID of the entity missing the component.
     */
    inline void error_entity_has_no_component(std::size_t entity_id) {
        std::cerr << "[ERROR] Entity " << entity_id
            << " does not have a component!" << std::endl;
    }

    /**
     * @brief Prints a formatted error when an entity already has a component.
     * @param entity_id ID of the entity that already has the component.
     */
    inline void error_entity_already_has_component(std::size_t entity_id) {
        std::cerr << "[ERROR] Entity " << entity_id
            << " already has a component!" << std::endl;
    }

    /**
     * @brief Checks if the entity_id is out of bounds of the sparse set.
     * @param entity_id ID of the entity being checked.
     * @return True if the entity_id is within bounds, false otherwise.
     */
    inline constexpr bool is_valid_entity_id(std::size_t entity_id) const noexcept {
        return entity_id < sparse_.size();
    }

public:
    /**
     * @brief Constructs the sparse set and reserves internal storage with the given capacities.
     * @param init_dense_capacity Initial capacity for the dense (default: DEFAULT_DENSE_CAPACITY).
     * @param init_sparse_capacity Initial capacity for the sparse (default: DEFAULT_SPARSE_CAPACITY).
     * @note The reverse_ array is also reserved with init_dense_capacity.
     */
    Sparse(std::size_t init_dense_capacity = DEFAULT_DENSE_CAPACITY,
        std::size_t init_sparse_capacity = DEFAULT_SPARSE_CAPACITY) {
        dense_.reserve(init_dense_capacity);
        reverse_.reserve(init_dense_capacity);
        sparse_.reserve(init_sparse_capacity);
        sparse_.resize(init_sparse_capacity, SIZE_MAX);
    }

    /**
     * @brief Default copy constructor.
     *
     * Performs a member-wise copy of the Sparse set.
     */
    Sparse(const Sparse&) = default;

    /**
     * @brief Default copy assignment operator.
     *
     * Performs member-wise assignment from another Sparse set.
     */
    Sparse& operator=(const Sparse&) = default;

    /**
     * @brief Default move constructor.
     *
     * Moves resources from another Sparse set. noexcept ensures no exceptions.
     */
    Sparse(Sparse&&) noexcept = default;

    /**
     * @brief Default move assignment operator.
     *
     * Moves resources from another Sparse set. noexcept ensures no exceptions.
     */
    Sparse& operator=(Sparse&&) noexcept = default;

    /**
     * @brief Default destructor.
     *
     * Cleans up resources. All members are automatically destroyed.
     */
    ~Sparse() = default;


public:
    /**
     * @brief Inserts a given component associated with a specific entity into the sparse set.
     * @param entity_id Id of the entity to associate the component with.
     * @param component The component to associate with the entity and insert into the sparse set.
     */
    template<typename U>
    void insert(std::size_t entity_id, U&& component) noexcept {
        if (!is_valid_entity_id(entity_id)) {
            error_not_enough_capacity(
                "Sparse vector too small",
                entity_id + 1,
                sparse_.size()
            ); return;
        }

        if (contains(entity_id)) {
            error_entity_already_has_component(entity_id);
            return;
        }

        dense_.push_back(std::forward<U>(component));
        std::size_t component_index = dense_.size() - 1;

        sparse_[entity_id] = component_index;
        reverse_.push_back(entity_id);
    }

    /**
     * @brief Inserts a single given component associated with multiple entities into the sparse set.
     * @param entity_ids 
     */
    template<typename... Ts, typename U>
    auto batch_insert(Ts... entity_ids, const U& component) noexcept ->
        std::enable_if_t<(is_index_type_v<Ts> && ...) && (sizeof...(Ts) > 0),
        void> { (insert(entity_ids, component), ...); }

    auto emplace_default(std::size_t entity_id) noexcept ->
        std::enable_if_t<std::is_default_constructible_v<T>, void> {
        if (!is_valid_entity_id(entity_id)) {
            error_not_enough_capacity(
                "Sparse vector too small",
                entity_id + 1,
                sparse_.size()
            ); return;
        }

        if (contains(entity_id)) {
            error_entity_already_has_component(entity_id);
            return;
        }

        dense_.emplace_back();
        std::size_t component_index = dense_.size() - 1;

        sparse_[entity_id] = component_index;
        reverse_.push_back(entity_id);
    }

    template<typename... Ts>
    auto batch_emplace(Ts... entity_ids) noexcept ->
        std::enable_if_t<std::is_default_constructible_v<T>
        && (is_index_type_v<Ts> && ...) && (sizeof...(Ts) > 0),
        void> { (emplace_default(entity_ids), ...); }

    void remove_swap(std::size_t entity_id) {
        if (!is_valid_entity_id(entity_id)) {
            error_not_enough_capacity(
                "Sparse vector too small",
                entity_id + 1,
                sparse_.size()
            ); return;
        }

        if (!contains(entity_id)) {
            error_entity_has_no_component(entity_id);
            return;
        }

        std::size_t component_index = sparse_[entity_id];
        std::size_t last_component_index = dense_.size() - 1;
        std::size_t last_entity_id = reverse_[last_component_index];

        std::swap(dense_[component_index], dense_[dense_.size() - 1]);
        dense_.pop_back();

        std::swap(reverse_[component_index], reverse_[last_component_index]);
        reverse_.pop_back();

        sparse_[last_entity_id] = component_index;
        sparse_[entity_id] = SIZE_MAX;
    }

    template<typename... Ts>
    auto batch_remove_swap(Ts... entity_ids) noexcept ->
        std::enable_if_t<((is_index_type_v<Ts>) && ...)
        && (sizeof...(Ts) > 0), void> { (remove_swap(entity_ids), ...); }

public:
    bool contains(std::size_t entity_id) const {
        return is_valid_entity_id(entity_id) && sparse_[entity_id] != SIZE_MAX;
    }

    template<typename... Ts>
    auto batch_contains(Ts... entity_ids) const noexcept ->
        std::enable_if_t<(is_index_type_v<Ts> && ...)
        && (sizeof...(Ts) > 0), bool> { return (contains(entity_ids) && ...); }

    std::size_t count() const noexcept { return dense_.size(); }
    std::size_t capacity() const noexcept { return sparse_.capacity(); }
    std::size_t empty() const noexcept { return sparse_.empty(); }

    void clear_sparse() noexcept { sparse_.clear(); }
    void clear_dense() noexcept { dense_.clear(); }
    void clear_binding() noexcept { reverse_.clear(); }

    void reserve(std::size_t new_capacity) noexcept { sparse_.reserve(new_capacity); }

    void shrink_sparse_to_fit() noexcept { sparse_.shrink_to_fit(); }
    void shrink_dense_to_fit() noexcept { dense_.shrink_to_fit(); }
    void shrink_binding_to_fit() noexcept { reverse_.shrink_to_fit(); }

public:
    auto begin() noexcept { return dense_.begin(); }
    auto end() noexcept { return dense_.end(); }
    auto begin() const noexcept { return dense_.begin(); }
    auto end() const noexcept { return dense_.end(); }

public:
    std::optional<T*> operator[](std::size_t entity_id) noexcept {
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return std::nullopt;
        }
        return &dense_[sparse_[entity_id]];
    }

    const std::optional<T*> operator[](std::size_t entity_id) const noexcept {
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return std::nullopt;
        }
        return &dense_[sparse_[entity_id]];
    }

    std::optional<T*> get(std::size_t entity_id) & noexcept {
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return std::nullopt;
        }
        return &dense_[sparse_[entity_id]];
    }

    std::optional<const T*> get(std::size_t entity_id) const& noexcept {
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return std::nullopt;
        }
        return &dense_[sparse_[entity_id]];
    }

private:
    std::vector<std::size_t> sparse_; // contient un entity_id -> component_id
    std::vector<std::size_t> reverse_; // contient un composante_id -> entity_id
    std::vector<T> dense_;  //contient un component_id -> component (T)
};
