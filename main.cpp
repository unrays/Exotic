// Copyright (c) December 2025 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file.

template<typename... Ts>
class Registry {
public:
    /**
     * @brief Constructs the Registry and initialize the internal storage with the given types.
     *
     * @note Each Ts must inherit from Component<T>.
     *       The internal storage (tuple of Sparse<Ts>) is created.
     */
    template<
        typename = std::enable_if_t<
        (std::is_base_of_v<Component<Ts>, Ts> && ...),
    void>> Registry() noexcept {
        std::cout << "[Registry Constructor] Created!" << std::endl;
        storage_ = std::make_tuple(Sparse<Ts>{}...);
    }

    /**
     * @brief Default copy constructor.
     *
     * Performs a member-wise copy of the Registry.
     */
    Registry(const Registry&) = default;

    /**
     * @brief Default copy assignment operator.
     *
     * Performs member-wise assignment from another Registry.
     */
    Registry& operator=(const Registry&) = default;

    /**
     * @brief Default move constructor.
     *
     * Moves resources from another Registry. noexcept ensures no exceptions.
     */
    Registry(Registry&&) noexcept = default;

    /**
     * @brief Default move assignment operator.
     *
     * Moves resources from another Registry. noexcept ensures no exceptions.
     */
    Registry& operator=(Registry&&) noexcept = default;

    /**
     * @brief Default destructor.
     *
     * Cleans up resources. All members are automatically destroyed.
     */
    ~Registry() = default;


public:
     /**
      * @brief Emplaces a component of type T in the sparse set to the given entity ID.
      *
      * @tparam T The component type (must inherit from Component<T>).
      * @param entity_id ID of the entity to which the component is added.
      */
    template<typename T>
    auto emplace(std::size_t entity_id) noexcept ->
    std::enable_if_t<std::is_base_of_v<Component<T>, T>, void> {
        std::cout << "[Registry emplace] Entering the emplace() fonction!" << std::endl;

        std::cout << std::boolalpha << contains_type<T>() << std::endl;

        std::get<Sparse<T>>(storage_).emplace_default(entity_id);
    }

    /**
     * @brief Emplaces a component of type T in the sparse set to multiple entity IDs.
     * 
     * @tparam T The component type (must inherit from Component<T>).
     * @param entity_ids Variadic pack of entity IDs to which the component is added.
     * 
     * @note All entities must be convertible to std::size_t. Passing zero IDs is not allowed.
     */
    template<typename T, typename... Ts>
    auto emplace_all(Ts&&... entity_ids) noexcept ->
        std::enable_if_t<std::is_base_of_v<Component<T>, T>
        && (std::is_convertible_v<Ts, std::size_t> && ...)
        && (sizeof...(Ts) > 0), void> {
            //(emplace<T>(std::forward<Ts>(entity_ids)), ...);
            std::get<Sparse<T>>(storage_).batch_emplace(entity_ids);
    }

    /**
     * @brief Retrieves the component of type T associated with the specified entity ID.
     *
     * @tparam T The component type (must inherit from Component<T>
     *           and be part of the Registry type list).
     * @param entity_id ID of the entity whose component is being required.
     *
     * @return A reference to the component of type T.
     */
    template<typename T>
    [[nodiscard]] auto get(std::size_t entity_id) noexcept ->
    std::enable_if_t<std::is_base_of_v<Component<T>, T>
    && std::disjunction_v<std::is_same<T, Ts>...>, T&> {
        std::cout << "[Registry get] Entering the get() fonction!" << std::endl;

        Sparse<T>& sparse = std::get<Sparse<T>>(storage_);
        return *sparse.get(entity_id); //garbage si rien, attention
    }

    /**
     * @brief Retrieves all the components of type T associated with all given entity IDs.
     * 
     * @tparam T The component type (must inherit from Component<T>
     *           and be part of the Registry type list).
     * @param entity_ids Variadic pack of entity IDs to which the component is added.
     *
     * @return A std::tuple of references to all components of type T.
     */
    template<typename T, typename... Ts>
    [[nodiscard]] auto get_all(Ts&&... entity_ids) noexcept ->
        std::enable_if_t<std::is_base_of_v<Component<T>, T>
        && (std::is_convertible_v<Ts, std::size_t> && ...)
        && (sizeof...(Ts) > 0), std::tuple<T&...>> {
            return std::tie(get<T>(entity_ids)...);
    }

public:
    /**
     * @brief Checks whether the registry contains the component type T.
     * 
     * @tparam T The type being checked (must inherit from Component<T>
     *           and be part of the Registry type list).
     * 
     * @return True if the registry contains the component type T, false otherwise.
     */
    template<typename T>
    constexpr auto contains_type() const noexcept ->
    std::enable_if_t<std::is_base_of_v<Component<T>, T>, bool> {
        return std::disjunction_v<std::is_same<T, Ts>...>;
        /* Pas tellement utile étant donné que j'ai deja SFINAE */
    }
        
private:
    /**
     * @brief Internal storage for all component sparse sets.
     */
    std::tuple<Sparse<Ts>...> storage_;
};