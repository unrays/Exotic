<div align="center" style="line-height:1.4">

<h1 style="margin-bottom: 0.2em;">Exotic</h1>
<h3 style="margin-top:0; margin-bottom:0.5em;">A template-driven C++ ECS</h3>

<p style="margin-top:0.2em; margin-bottom:0.5em; font-weight:bold;">
Template-based, cache-friendly ECS designed for speed, scalability, and massive entity counts.
</p>

<p style="margin-top:0; color:#555;">November 28, 2025</p>

</div>

<hr style="border:0; border-top:1px solid #ccc; margin: 1em 0;">

I literally could have called it Wave 2.0, but I was a bit too lazy to have to justify myself and make the world understand that there was a version 1, so I chose another name that I think is super cool. This project, as of Friday, November 28, 2025, is one of my first real experiments with sfinae features and srtp, like std::enable_if... for example. Furthermore, I intend to add lots of super cool stuff in the future, so stay tuned.

I also want to mention that I've tried my best to avoid looking at popular ECS like Beavy, EnTT, or Flecs in order to maintain a "healthy" direction and learn as much as possible without being influenced by "best" practices. I enjoy making mistakes, failing, and getting back up; that's what I find fun about learning. If I watched and copied others, I simply wouldn't learn. Here, I'm quite proud of the TagsGroup system I've come up with; a component has one or more tags that it automatically assigns to its entity when the tag is applied. Basically, I want a dynamic tagging system. For example, if I add the Position component, the entity is automatically (by default) assigned the position tags. Finally, there's always the option to manually disable, add, or remove tags to control everything in very specific situations.

This is, in a way, the ultimate challenge of the last two months: learning this new passion, ECS. I'm still young, I still have a long way to go, but I feel like I've improved in the last few weeks (for example, the metaprogramming template over the last three weeks), and I need to consolidate all this learning into one ultimate project before 2026! Hoping this interests you and that you can understand my perspective, I'd like to discuss these topics, but I don't know anyone who likes ECS in my circle, so if you're curious, feel free to contact me :)

---

*Under construction*<br/>
*I'm currently working on a small personal engine called Mixin that implements parts of this ECS. As I develop it, I'll improve the borrowed ECS from here, and when I'm satisfied and convinced of its functionality in real-world situations, I'll post it here.*

## The cleanest and most concise version to date, really promising.
#### I re-started from scratch, that's why it's so slow going.
```cpp
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
      * 
      * @note The component must be registered in the registry.
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
     * @note The component must be registered in the registry.
     * @note All entity_ids must be unsigned integers.
     * @note Passing zero IDs is not allowed.
     */
    template<typename T, typename... Ids>
    auto emplace_all(Ids&&... entity_ids) noexcept ->
        std::enable_if_t<std::is_base_of_v<Component<T>, T>
        && ((std::is_integral_v<Ids> && std::is_unsigned_v<Ids>) && ...)
        && (sizeof...(Ids) > 0), void> {
            std::get<Sparse<T>>(storage_) //faire fonction get interne
                .batch_emplace(std::forward<Ids>(entity_ids)...);
    }

    /**
     * @brief Retrieves the component of type T associated with the specified entity ID.
     *
     * @tparam T The component type (must inherit from Component<T>
     *           and be part of the Registry type list).
     * @param entity_id ID of the entity whose component is being required.
     *
     * @return A reference to the component of type T.
     * 
     * @note The component must be registered in the registry.
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
     * 
     * @note The component must be registered in the registry.
     * @note All entity_ids must be unsigned integers.
     * @note Passing zero IDs is not allowed.
     */
    template<typename T, typename... Ids>
    [[nodiscard]] auto get_all(Ids&&... entity_ids) noexcept ->
        std::enable_if_t<std::is_base_of_v<Component<T>, T>
        && ((std::is_integral_v<Ids> && std::is_unsigned_v<Ids>) && ...)
        && std::disjunction_v<std::is_same<T, Ts>...>
        && (sizeof...(Ids) > 0), std::tuple<T&>> {
            return std::tie(get<T>(entity_ids)...);
    }

    /**
     * @brief Removes the component of type T associated with the given entity ID.
     * 
     * @tparam T The component type (must inherit from Component<T>).
     * @param entity_id ID of the entity whose component is being required.
     *
     * @note The component must be registered in the registry.
     */
    template<typename T>
    auto remove(std::size_t entity_id) noexcept ->
        std::enable_if_t<std::is_base_of_v<Component<T>, T>
        && std::disjunction_v<std::is_same<T, Ts>...>, void> {
            std::get<Sparse<T>>(storage_).remove_swap(entity_id);
    }

    /**
     * @brief Removes the component of type T associated with all given entitiy IDs.
     *
     * @tparam T The component type (must inherit from Component<T>).
     * @param entity_ids Variadic pack of entity IDs to which the component is added.
     *
     * @note The component must be registered in the registry.
     * @note All entity_ids must be unsigned integers.
     * @note Passing zero IDs is not allowed.
     */
    template<typename T, typename... Ids>
    auto remove_all(Ids&&... entity_ids) noexcept ->
        std::enable_if_t<std::is_base_of_v<Component<T>, T>
        && std::disjunction_v<std::is_same<T, Ts>...>
        && ((std::is_integral_v<Ids> && std::is_unsigned_v<Ids>) && ...)
        && (sizeof...(Ids) > 0), void> {
            std::get<Sparse<T>>(storage_)
                .batch_remove_swap(std::forward<Ids>(entity_ids)...);
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
```

## I'm very proud of :) 
```cpp
// Copyright (c) December 2025 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file.

template<typename T>
class Sparse : public ISparseSet{
private:
    static constexpr std::size_t DEFAULT_DENSE_CAPACITY = 2048;
    static constexpr std::size_t DEFAULT_SPARSE_CAPACITY = 16384;

    using ContainedType = T;

    /**
     * @brief Prints a formatted error for insufficient capacity.
     * 
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
     * 
     * @param entity_id ID of the entity missing the component.
     */
    inline void error_entity_has_no_component(std::size_t entity_id) {
        std::cerr << "[ERROR] Entity " << entity_id
            << " does not have a component!" << std::endl;
    }

    /**
     * @brief Prints a formatted error when an entity already has a component.
     * 
     * @param entity_id ID of the entity that already has the component.
     */
    inline void error_entity_already_has_component(std::size_t entity_id) {
        std::cerr << "[ERROR] Entity " << entity_id
            << " already has a component!" << std::endl;
    }

    /**
     * @brief Checks if the entity_id is out of bounds of the sparse set.
     * 
     * @param entity_id ID of the entity being checked.
     * 
     * @return True if the entity_id is within bounds, false otherwise.
     */
    inline constexpr bool is_valid_entity_id(std::size_t entity_id) const noexcept {
        return entity_id < sparse_.size();
    }

public:
    /**
     * @brief Constructs the sparse set and reserves internal storage with the given capacities.
     * 
     * @param init_dense_capacity Initial capacity for the dense (default: DEFAULT_DENSE_CAPACITY).
     * @param init_sparse_capacity Initial capacity for the sparse (default: DEFAULT_SPARSE_CAPACITY).
     * 
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
     * 
     * @param entity_id Id of the entity to associate the component with.
     * @param component The component to associate with the entity and insert into the sparse set.
     *
     * @note The component must be the same as the type stored.
     */
    template<typename U>
    auto insert(std::size_t entity_id, const U& component) noexcept ->
    std::enable_if_t<std::is_same_v<U, T>, void> {
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

        dense_.push_back(component);
        std::size_t component_index = dense_.size() - 1;

        sparse_[entity_id] = component_index;
        reverse_.push_back(entity_id);
    }

    /**
     * @brief Inserts a single given component associated with multiple entities into the sparse set.
     *
     * @param entity_ids Ids of all the entities to associate the component with.
     * @param component The component to associate with all the entities and insert into the sparse set.
     * 
     * @note All entity_ids must be unsigned integers.
     * @note The component must be the same as the type stored.
     * @note Requires at least one entity_id.
     */
    template<typename U, typename... Ts>
    auto batch_insert(Ts&&... entity_ids, const U& component) noexcept ->
        std::enable_if_t<std::is_same_v<U, T> && (std::is_integral_v<Ts> && ...)
        && (std::is_unsigned_v<Ts> && ...) && (sizeof...(Ts) > 0), void> {
            (insert(std::forward<Ts>(entity_ids), component), ...);
    }

    /**
     * @brief Emplaces a type T component in the sparse set
     *        associated with the given entity.
     *
     * @param entity_id Id of the entity to associate the component with.
     */
    auto emplace_default(std::size_t entity_id) noexcept ->
        std::enable_if_t<std::is_default_constructible_v<T>, void> {
        if (!is_valid_entity_id(entity_id)) {
            error_not_enough_capacity(
                "Sparse vector too small",
                entity_id + 1,
                sparse_.size()
            ); return;
        }

        std::cout << "Emplacing new component for entity " << entity_id << std::endl;

        if (contains(entity_id)) {
            error_entity_already_has_component(entity_id);
            return;
        }

        dense_.emplace_back();
        std::size_t component_index = dense_.size() - 1;

        sparse_[entity_id] = component_index;
        reverse_.push_back(entity_id);
    }

    /**
     * @brief Emplaces a type T component in the sparse set
     *        associated with multiple entities into the sparse set.
     *
     * @param entity_ids Ids of all the entities to associate the component with.
     * 
     * @note All entity_ids must be unsigned integers.
     * @note Requires at least one entity_id.
     */
    template<typename... Ts>
    auto batch_emplace(Ts&&... entity_ids) noexcept ->
        std::enable_if_t<std::is_default_constructible_v<T>
        && (std::is_integral_v<Ts> && ...) && (std::is_unsigned_v<Ts> && ...)
        && (sizeof...(Ts) > 0) ,void> {
            (emplace_default(std::forward<Ts>(entity_ids)), ...);
    }

    /**
     * @brief Removes an entity from the sparse set using swap-and-pop strategy.
     * 
     * @param entity_ids Id of the entity to associate the component with.
     */
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

    /**
     * @brief Removes an entity from the sparse set using swap-and-pop strategy.
     *
     * @param entity_ids Ids of all the entities to associate the component with.
     *
     * @note All entity_ids must be unsigned integers.
     * @note Requires at least one entity_id.
     */
    template<typename... Ts>
    auto batch_remove_swap(Ts&&... entity_ids) noexcept ->
        std::enable_if_t<(std::is_integral_v<Ts> && ...)
        && (std::is_unsigned_v<Ts> && ...) && (sizeof...(Ts) > 0), void> {
            (remove_swap(std::forward<Ts>(entity_ids)), ...);
    }

public:
    bool contains(std::size_t entity_id) const {
        return is_valid_entity_id(entity_id) && sparse_[entity_id] != SIZE_MAX;
    }

    template<typename... Ts>
    auto batch_contains(Ts&&... entity_ids) const noexcept ->
        std::enable_if_t<(std::is_integral_v<Ts> && ...)
        && (std::is_unsigned_v<Ts> && ...) && (sizeof...(Ts) > 0), bool> {
            return (contains(std::forward<Ts>(entity_ids)) && ...);
    }

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
    T* operator[](std::size_t entity_id) noexcept { // NOTE: unsafe
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return nullptr;
        }
        return &dense_[sparse_[entity_id]];
    }

    const T* operator[](std::size_t entity_id) const noexcept { // NOTE: unsafe
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return nullptr;
        }
        return &dense_[sparse_[entity_id]];
    }

    T* get(std::size_t entity_id) & noexcept { // NOTE: unsafe
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return nullptr;
        }
        return &dense_[sparse_[entity_id]];
    }

    const T* get(std::size_t entity_id) const& noexcept { // NOTE: unsafe
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return nullptr;
        }
        return &dense_[sparse_[entity_id]];
    }

private:
    std::vector<std::size_t> sparse_; // contient un entity_id -> component_id
    std::vector<std::size_t> reverse_; // contient un composante_id -> entity_id
    std::vector<T> dense_;  //contient un component_id -> component (T)
};
```

## First view system of my career :) *(it's not perfect, far from it)*
```cpp
// Copyright (c) December 2025 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file.

template<typename... Ts>
class View {
public:
    /**
     * @brief Constructs the view by initializing the internal storage
     *        with the given references of existing sparse sets
     *
     * @note ... (sfinae and conditions)
     */
    View(Sparse<Ts>&... sparses) noexcept
        : s_ref_(sparses...) {
    }

    //faut interdir l'instanciation et plus
    //uniquement le registry qui doit etre en mesure

    /**
     * @brief Default copy constructor.
     *
     * Performs a member-wise copy of the View.
     */
    View(const View&) = default;

    /**
     * @brief Default copy assignment operator.
     *
     * Performs member-wise assignment from another View.
     */
    View& operator=(const View&) = default;

    /**
     * @brief Default move constructor.
     *
     * Moves resources from another View. noexcept ensures no exceptions.
     */
    View(View&&) noexcept = default;

    /**
     * @brief Default move assignment operator.
     *
     * Moves resources from another View. noexcept ensures no exceptions.
     */
    View& operator=(View&&) noexcept = default;

    /**
     * @brief Default destructor.
     *
     * Cleans up resources. All members are automatically destroyed.
     */
    ~View() = default;
    
public:
    class ViewIterator { //en gros, ++ et si entity na pas les trois, il skil au prochain
    public:
        /**
         * @brief Constructs a ViewIterator with the specified initial and end state.
         * 
         * @param begin_id The starting entity ID for the iterator.
         * @param end_id The ending entity ID for the iterator.
         * 
         * genre verif si begin est plus petit que end
         */ 
        ViewIterator(std::tuple<Sparse<Ts>&...>& ts_ref, std::size_t begin_id, std::size_t max_id) noexcept
            : ts_ref_(ts_ref), current_id_(begin_id), end_id_(max_id) {}


        /**
         * @brief Default copy constructor.
         *
         * Performs a member-wise copy of the ViewIterator.
         */
        ViewIterator(const ViewIterator&) = default;

        /**
         * @brief Default copy assignment operator.
         *
         * Performs member-wise assignment from another ViewIterator.
         */
        ViewIterator& operator=(const ViewIterator&) = default;

        /**
         * @brief Default move constructor.
         *
         * Moves resources from another ViewIterator. noexcept ensures no exceptions.
         */
        ViewIterator(ViewIterator&&) noexcept = default;

        /**
         * @brief Default move assignment operator.
         *
         * Moves resources from another ViewIterator. noexcept ensures no exceptions.
         */
        ViewIterator& operator=(ViewIterator&&) noexcept = default;

        /**
         * @brief Default destructor.
         *
         * Cleans up resources. All members are automatically destroyed.
         */
        ~ViewIterator() = default;

    public:
        /**
         * @brief Returns the current entity ID.
         *
         * @return The ID of the entity the iterator currently points to.
         */
        std::size_t operator*() const { return current_id_; }

        /**
         * @brief Advances the iterator to the next element (prefix version).
         *
         * Moves the iterator forward to the next valid element according to the container's rules.
         *
         * @return A reference to the iterator after it has been advanced.
         *
         * @note This function modifies the iterator in-place.
         */
        ViewIterator& operator++() {
            while (current_id_ != end_id_ && !is_valid_entity(++current_id_));
            return *this;
        }

        /**
         * @brief Advances the iterator to the next element (postfix version).
         *
         * Moves the iterator forward to the next valid element, but returns the iterator state
         * prior to the increment.
         *
         * @param int Dummy parameter to distinguish from prefix increment.
         * @return A copy of the iterator before it was advanced.
         *
         * @note This version may be less efficient than prefix because it typically involves copying.
         */
        ViewIterator operator++(int) {
            ViewIterator temp = *this;
            while (current_id_ != end_id_ && !is_valid_entity(++current_id_));
            return temp;
        }

        /**
         * @brief Compares two iterators for equality.
         *
         * Two iterators are considered equal if they point to the same entity (current_id_).
         *
         * @param other The iterator to compare with.
         * @return true if both iterators point to the same entity, false otherwise.
         */
        bool operator==(const ViewIterator& other) const noexcept {
            return current_id_ == other.current_id_;
        }

        //faire un genre de it-> (ts...) choisi la composante de l'itérateur

        /**
         * @brief Compares two iterators for inequality.
         *
         * Two iterators are considered unequal if they point to different entities.
         *
         * @param other The iterator to compare with.
         * @return true if the iterators point to different entities, false otherwise.
         */
        bool operator!=(const ViewIterator& other) const noexcept {
            return current_id_ != other.current_id_;
        }

    private:
        /**
         * @brief Checks whether the given entity exists in all associated sparse sets.
         *
         * @param entity_id ID of the entity whose component is being required.
         * 
         * @return True if the given entity exists in all associated sparse sets.
         *         False otherwise.
         */
        inline constexpr bool is_valid_entity(std::size_t entity_id) const noexcept {
            return std::apply([&](auto&&... args) {
                return (((args[entity_id] != nullptr) && ...)); // temporary patch
            }, ts_ref_);
        }

    private:
        std::size_t current_id_;
        std::size_t end_id_;

    private:
        std::tuple<Sparse<Ts>&...>& ts_ref_;
    };

    /* result espéré */
    /*int arr[3] = {1,2,3};

    MyIterator begin{arr};
    MyIterator end{arr + 3};

    for (auto it = begin; it != end; ++it) {
        std::cout << *it << "\n";
    }*/

    /*for (auto it = view.begin(); it != view.end(); ++it) { ... }*/

    /* auto view = registry.view<Position, Velocity, Health>();
       le registre injecte les sparse set correspondants dans view */

public:
    template<typename F>
    auto each(F&& func) noexcept ->
    std::enable_if_t<std::is_invocable_v<F, Ts&...>, void> {
        std::apply([&](auto&&... args) {
            for (auto it = begin(); it != end(); ++it) {
                func(*args[*it]...);
            }
        }, s_ref_);
    }

    /**
     * @brief Returns a ViewIterator starting at index 0.
     * 
     * @return A ViewIterator at index 0.
     */
    ViewIterator begin() noexcept {
        std::size_t max = std::apply([](auto&&... args) {
            return std::max({ args.count()... });
        }, s_ref_);
        return ViewIterator(s_ref_, DEFAULT_MIN_SIZE, max);
    }

    /**
     * @brief Returns a ViewIterator starting at highest index among stored sets.
     *
     * @return A ViewIterator representing the end of iteration.
     */
    ViewIterator end() noexcept {
        std::size_t max = std::apply([](auto&&... args) {
            return std::max({ args.count()... });
        }, s_ref_);
        return ViewIterator(s_ref_, max, max);
    }

private:
    static constexpr std::size_t DEFAULT_MIN_SIZE = 0;

private:
    std::tuple<Sparse<Ts>&...> s_ref_;
};
```

## A interesting prototype, but not viable enough for large-scale deployment.
```cpp
// Copyright (c) December 2025 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file.

#pragma once
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <tuple>


namespace Tags {
    struct Colored {};
    struct Transparent {};
    struct Visible {};
    struct ShadowCaster {};
    struct ShadowReceiver {};
    struct Animated {};
    struct Sprite {};
    struct Mesh {};
    struct LightSource {};
    struct ParticleEmitter {};

    struct Rigid {};
    struct Movable {};
    struct Physics {};
    struct Collidable {};
    struct Trigger {};
    struct GravityAffected {};
    struct Kinematic {};
    struct Projectile {};
    struct Pickup {};
    struct Destructible {};
    struct Interactable {};

    struct AIControlled {};
    struct Pathfinding {};
    struct Patrol {};
    struct Attackable {};
    struct Friendly {};
    struct Hostile {};
    struct NPC {};
    struct Boss {};
    struct QuestObjective {};

    struct InputSensitive {};
    struct PlayerControlled {};
    struct Networked {};
    struct LocalPlayer {};
    struct RemotePlayer {};

    struct Persistent {};
    struct Temporary {};
    struct Dynamic {};
    struct Static {};
    struct Debug {};
    struct EditorOnly {};

    struct Replicated {};
    struct SyncPosition {};
    struct SyncState {};
    struct SyncAnimation {};

    struct SoundEmitter {};
    struct MusicEmitter {};
    struct AudioListener {};

    struct DisabledTag {};

    template<typename T>
    struct is_tag : std::false_type {};

    template<typename T>
    inline constexpr bool is_tag_v = is_tag<T>::value;

#define REGISTER_TAG(T) template<> struct is_tag<T> : std::true_type {}
    REGISTER_TAG(Colored);
    REGISTER_TAG(Transparent);
    REGISTER_TAG(Visible);
    REGISTER_TAG(ShadowCaster);
    REGISTER_TAG(ShadowReceiver);
    REGISTER_TAG(Animated);
    REGISTER_TAG(Sprite);
    REGISTER_TAG(Mesh);
    REGISTER_TAG(LightSource);
    REGISTER_TAG(ParticleEmitter);

    REGISTER_TAG(Rigid);
    REGISTER_TAG(Movable);
    REGISTER_TAG(Physics);
    REGISTER_TAG(Collidable);
    REGISTER_TAG(Trigger);
    REGISTER_TAG(GravityAffected);
    REGISTER_TAG(Kinematic);
    REGISTER_TAG(Projectile);
    REGISTER_TAG(Pickup);
    REGISTER_TAG(Destructible);
    REGISTER_TAG(Interactable);

    REGISTER_TAG(AIControlled);
    REGISTER_TAG(Pathfinding);
    REGISTER_TAG(Patrol);
    REGISTER_TAG(Attackable);
    REGISTER_TAG(Friendly);
    REGISTER_TAG(Hostile);
    REGISTER_TAG(NPC);
    REGISTER_TAG(Boss);
    REGISTER_TAG(QuestObjective);

    REGISTER_TAG(InputSensitive);
    REGISTER_TAG(PlayerControlled);
    REGISTER_TAG(Networked);
    REGISTER_TAG(LocalPlayer);
    REGISTER_TAG(RemotePlayer);

    REGISTER_TAG(Persistent);
    REGISTER_TAG(Temporary);
    REGISTER_TAG(Dynamic);
    REGISTER_TAG(Static);
    REGISTER_TAG(Debug);
    REGISTER_TAG(EditorOnly);

    REGISTER_TAG(Replicated);
    REGISTER_TAG(SyncPosition);
    REGISTER_TAG(SyncState);
    REGISTER_TAG(SyncAnimation);

    REGISTER_TAG(SoundEmitter);
    REGISTER_TAG(MusicEmitter);
    REGISTER_TAG(AudioListener);
#undef REGISTER_TAG
}

template <typename Derived>
struct Component {
    //using BaseType = Component<T>;
    void test() {
        printf("Just for testing purposes :)");
    }

    auto self() {
        //poubelle
        return static_cast<Derived*>(this);
    }


};

template<typename... Ts>
struct TagsGroup {
    //static_assert((Tags::is_tag<Ts>::value && ...), "All types must be valid tags");
    static_assert((Tags::is_tag_v<Ts> && ...), "All types must be valid tags");



    void test2() {
        printf("Just for testing purposes :)");
    }
};

namespace Components {
    namespace Internal {
        struct raw_position { std::uint32_t x, y; };
        struct raw_velocity { std::uint32_t vx, vy; };
        struct raw_rotation { std::uint8_t angle; };
        struct raw_scale { std::uint8_t sx, sy; };
        struct raw_color { std::uint8_t r, g, b, a; };
    }

    template<typename T>
    struct is_type : std::false_type {};

    template<typename T>
    static constexpr bool is_component_v = is_type<T>::value;

    //genre mettre des utilitaires comme same component
    //ou plus whatever

    struct Position : public Component <Position>,
        public TagsGroup<Tags::Movable, Tags::Physics>,
        public Internal::raw_position {
        void operator()(std::uint32_t x, std::uint32_t y) {
            this->x = x; this->y = y;
        }
    };

    //mettre les using ici avec les tags et le srtp de component<derived>

#define REGISTER_COMPONENT(T) template<> struct is_type<T> : std::true_type {} 
    REGISTER_COMPONENT(Position);
    REGISTER_COMPONENT(Internal::raw_velocity);
    REGISTER_COMPONENT(Internal::raw_rotation);
    REGISTER_COMPONENT(Internal::raw_scale);
    REGISTER_COMPONENT(Internal::raw_color);
    //prob remplacer par genre les struct externes directement
#undef REGISTER_COMPONENT
}

class EntityId {
private:
    static std::uint32_t _nextId;

public:
    static std::uint32_t Next() { return _nextId++; }
};

std::uint32_t EntityId::_nextId = 0;

struct Entity {
    std::uint32_t Value;

    Entity() : Value(EntityId::Next()) {}
};

struct Position : public Component<Position>,
    public TagsGroup<Tags::Movable, Tags::SyncPosition>
{
    std::uint32_t X, Y;
};

struct Velocity : public Component<Velocity>,
    public TagsGroup<Tags::Movable, Tags::Physics>
{
    std::uint32_t VX, VY;
};

struct Rotation : public Component<Rotation>,
    public TagsGroup<Tags::Physics, Tags::SyncState>
{
    std::uint8_t Angle;
};

struct Scale : public Component<Scale>,
    public TagsGroup<Tags::Visible>
{
    std::uint8_t X, Y;
};

struct Color : public Component<Color>,
    public TagsGroup<Tags::Colored, Tags::Visible>
{
    std::uint8_t R, G, B, A;
};

class Registry {
private:
    struct Constants {
        static constexpr std::uint32_t InitialEntityCapacity = 131072;
        static constexpr std::uint32_t InitialPoolCapacity = 262143;
    };
    struct Variables {
        std::vector<Position> _positions;
        std::vector<int> _entityToPosIndex;

        std::vector<Velocity> _velocities;
        std::vector<int> _entityToVelIndex;

        std::vector<Rotation> _rotations;
        std::vector<int> _entityToRotIndex;

        std::vector<Scale> _scales;
        std::vector<int> _entityToScaleIndex;

        std::vector<Color> _colors;
        std::vector<int> _entityToColorIndex;
    };

    //std::unordered_map<std::type_index, std::unique_ptr<void>> pools;

private:
    Constants constants;
    Variables variables;

private:
    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using szt = std::size_t;

private:
    template<typename T>
    auto internal_add_component(std::uint32_t eidx) noexcept ->
        std::enable_if_t<std::is_class_v<T>
        && std::is_base_of_v<Component<T>, T>,
        void> /* Internal interface for adding components */ {
        std::cout << "[WXR Component] Added " << typeid(T).name() << " to Entity " << eidx << std::endl;

        //peut etre tuple pool ou whatever avec des types et ajouter un type
        //a la pool a chaque ajout de "nouveau" component et faire un unpacking 
        //du tuple<typename... Ts> et faire la lambda if constexpr... std::same...
        //pour chacun.

        if constexpr (std::is_same_v<T, Position>) {
            if (variables._entityToPosIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Position component\n", eidx);
                return;
            }
            variables._entityToPosIndex[eidx] = variables._positions.size();
            variables._positions.emplace_back(Position{});
        }
        else if constexpr (std::is_same_v<T, Velocity>) {
            if (variables._entityToVelIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Velocity component\n", eidx);
                return;
            }
            variables._entityToVelIndex[eidx] = variables._velocities.size();
            variables._velocities.emplace_back(Velocity{});
        }
        else if constexpr (std::is_same_v<T, Rotation>) {
            if (variables._entityToRotIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Rotation component\n", eidx);
                return;
            }
            variables._entityToRotIndex[eidx] = variables._rotations.size();
            variables._rotations.emplace_back(Rotation{});
        }
        else if constexpr (std::is_same_v<T, Scale>) {
            if (variables._entityToScaleIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Scale component\n", eidx);
                return;
            }
            variables._entityToScaleIndex[eidx] = variables._scales.size();
            variables._scales.emplace_back(Scale{});
        }
        else if constexpr (std::is_same_v<T, Color>) {
            if (variables._entityToColorIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Color component\n", eidx);
                return;
            }
            variables._entityToColorIndex[eidx] = variables._colors.size();
            variables._colors.emplace_back(Color{});
        }
    }

    void internal_get_component() noexcept {

    }

    void internal_remove_component() noexcept {

    }

public:
    Registry(std::size_t maxEntities, std::size_t maxPool) {
        variables._entityToPosIndex.resize(maxEntities, -1);
        variables._entityToVelIndex.resize(maxEntities, -1);
        variables._entityToRotIndex.resize(maxEntities, -1);
        variables._entityToScaleIndex.resize(maxEntities, -1);
        variables._entityToColorIndex.resize(maxEntities, -1);

        variables._positions.reserve(maxPool);
        variables._velocities.reserve(maxPool);
        variables._rotations.reserve(maxPool);
        variables._scales.reserve(maxPool);
        variables._colors.reserve(maxPool);
    }


    template<typename... Ts>
    auto Add(std::uint32_t eidx) noexcept ->
        std::enable_if_t<(std::is_class_v<Ts> && ...) &&
        (std::is_base_of_v<Component<Ts>, Ts> && ...),
        void> /* Serves as a entry point for adding components */ {
        ((internal_add_component<Ts>(eidx)), ...);
    }

    template<typename... Ts>
    auto AddTag(std::uint32_t eidx) noexcept ->
        std::enable_if_t<(
            (Tags::is_tag_v<Ts>), ...),
        void> /* Securly add tags to an entity */ {
        ((std::cout << "[WXR Tag] Added "
            << typeid(Ts).name()
            << " to Entity "
            << eidx << std::endl)
            , ...);
    }


    template <typename T> void Add(std::uint32_t eidx) {
        if constexpr (std::is_same_v<T, Position>) {
            if (variables._entityToPosIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Position component\n", eidx);
                return;
            }
            variables._entityToPosIndex[eidx] = variables._positions.size();
            variables._positions.emplace_back(Position{});
        }
        else if constexpr (std::is_same_v<T, Velocity>) {
            if (variables._entityToVelIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Velocity component\n", eidx);
                return;
            }
            variables._entityToVelIndex[eidx] = variables._velocities.size();
            variables._velocities.emplace_back(Velocity{});
        }
        else if constexpr (std::is_same_v<T, Rotation>) {
            if (variables._entityToRotIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Rotation component\n", eidx);
                return;
            }
            variables._entityToRotIndex[eidx] = variables._rotations.size();
            variables._rotations.emplace_back(Rotation{});
        }
        else if constexpr (std::is_same_v<T, Scale>) {
            if (variables._entityToScaleIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Scale component\n", eidx);
                return;
            }
            variables._entityToScaleIndex[eidx] = variables._scales.size();
            variables._scales.emplace_back(Scale{});
        }
        else if constexpr (std::is_same_v<T, Color>) {
            if (variables._entityToColorIndex[eidx] != -1) {
                printf("[ERROR] Entity %u already has Color component\n", eidx);
                return;
            }
            variables._entityToColorIndex[eidx] = variables._colors.size();
            variables._colors.emplace_back(Color{});
        }
    }


    template<typename T> T& Get(std::uint32_t eidx) {
        if constexpr (std::is_same_v<T, Position>) {
            if (variables._entityToPosIndex[eidx] == -1)
                throw std::runtime_error(
                    "Entity " + std::to_string(eidx) + " does NOT have Position component"
                );
            return variables._positions[variables._entityToPosIndex[eidx]];
        }
        else if constexpr (std::is_same_v<T, Velocity>) {
            if (variables._entityToVelIndex[eidx] == -1)
                throw std::runtime_error(
                    "Entity " + std::to_string(eidx) + " does NOT have Velocity component"
                );
            return variables._velocities[variables._entityToVelIndex[eidx]];
        }
        else if constexpr (std::is_same_v<T, Rotation>) {
            if (variables._entityToRotIndex[eidx] == -1)
                throw std::runtime_error(
                    "Entity " + std::to_string(eidx) + " does NOT have Rotation component"
                );
            return variables._rotations[variables._entityToRotIndex[eidx]];
        }
        else if constexpr (std::is_same_v<T, Scale>) {
            if (variables._entityToScaleIndex[eidx] == -1)
                throw std::runtime_error(
                    "Entity " + std::to_string(eidx) + " does NOT have Scale component"
                );
            return variables._scales[variables._entityToScaleIndex[eidx]];
        }
        else if constexpr (std::is_same_v<T, Color>) {
            if (variables._entityToColorIndex[eidx] == -1)
                throw std::runtime_error(
                    "Entity " + std::to_string(eidx) + " does NOT have Color component"
                );
            return variables._colors[variables._entityToColorIndex[eidx]];
        }
        throw std::runtime_error("Component type not supported");
    }
};

int main() {
    std::cout << "Hello World!\n";

    constexpr size_t entityCount = 100000;
    constexpr size_t maxPool = 200000;

    Registry registry(entityCount, maxPool);

    registry.Add<Position, Velocity>(0);
    registry.Add<Position, Velocity>(1);

    registry.AddTag<Tags::Debug, Tags::NPC>(0);

    registry.AddTag<Tags::Colored>(0);
    registry.AddTag<Tags::Colored>(1);

    auto pos = registry.Get<Position>(0);
    pos.X = 2;

    std::cout << pos.X << std::endl;

    Components::Position position;

    position.x = 5;

    position(25, 50);

    std::cout << position.x << ", " << position.y << std::endl;
}
```

## Latest code
```cpp
// Copyright (c) December 2025 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file.

// Copyright (c) December 2025 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file.

#include <iostream>
#include <vector>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <tuple>
#include <cassert>
#include <optional>
#include <unordered_map>
#include <typeindex>

//#include "ecs/Sparse.hpp"
#include "utils/isIndexType.hpp"


template<typename Derived>
struct Component {
public:
    auto test() const noexcept {
        std::cout << "Method call from 'Component' base." << std::endl;
    }

private:

};

struct DummyComponent : public Component<DummyComponent> {
    std::size_t value;
};

struct Position : public Component<Position> {
    std::uint32_t x, y;
};

struct Velocity : public Component<Velocity> {
    std::uint32_t vx, vy;
};

struct ISparseSet {
    virtual ~ISparseSet() = default;
};

// TODO: Refaire les conditions SFINAE afin d'empêcher les insertions indésirables
template<typename T>
class Sparse : public ISparseSet{
private:
    static constexpr std::size_t DEFAULT_DENSE_CAPACITY = 2048;
    static constexpr std::size_t DEFAULT_SPARSE_CAPACITY = 16384;

    using ContainedType = T;

    /**
     * @brief Prints a formatted error for insufficient capacity.
     * 
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
     * 
     * @param entity_id ID of the entity missing the component.
     */
    inline void error_entity_has_no_component(std::size_t entity_id) {
        std::cerr << "[ERROR] Entity " << entity_id
            << " does not have a component!" << std::endl;
    }

    /**
     * @brief Prints a formatted error when an entity already has a component.
     * 
     * @param entity_id ID of the entity that already has the component.
     */
    inline void error_entity_already_has_component(std::size_t entity_id) {
        std::cerr << "[ERROR] Entity " << entity_id
            << " already has a component!" << std::endl;
    }

    /**
     * @brief Checks if the entity_id is out of bounds of the sparse set.
     * 
     * @param entity_id ID of the entity being checked.
     * 
     * @return True if the entity_id is within bounds, false otherwise.
     */
    inline constexpr bool is_valid_entity_id(std::size_t entity_id) const noexcept {
        return entity_id < sparse_.size();
    }

public:
    /**
     * @brief Constructs the sparse set and reserves internal storage with the given capacities.
     * 
     * @param init_dense_capacity Initial capacity for the dense (default: DEFAULT_DENSE_CAPACITY).
     * @param init_sparse_capacity Initial capacity for the sparse (default: DEFAULT_SPARSE_CAPACITY).
     * 
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
     * 
     * @param entity_id Id of the entity to associate the component with.
     * @param component The component to associate with the entity and insert into the sparse set.
     *
     * @note The component must be the same as the type stored.
     */
    template<typename U>
    auto insert(std::size_t entity_id, const U& component) noexcept ->
    std::enable_if_t<std::is_same_v<U, T>, void> {
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

        dense_.push_back(component);
        std::size_t component_index = dense_.size() - 1;

        sparse_[entity_id] = component_index;
        reverse_.push_back(entity_id);
    }

    /**
     * @brief Inserts a single given component associated with multiple entities into the sparse set.
     *
     * @param entity_ids Ids of all the entities to associate the component with.
     * @param component The component to associate with all the entities and insert into the sparse set.
     * 
     * @note All entity_ids must be unsigned integers.
     * @note The component must be the same as the type stored.
     * @note Requires at least one entity_id.
     */
    template<typename U, typename... Ts>
    auto batch_insert(Ts&&... entity_ids, const U& component) noexcept ->
        std::enable_if_t<std::is_same_v<U, T> && (std::is_integral_v<Ts> && ...)
        && (std::is_unsigned_v<Ts> && ...) && (sizeof...(Ts) > 0), void> {
            (insert(std::forward<Ts>(entity_ids), component), ...);
    }

    /**
     * @brief Emplaces a type T component in the sparse set
     *        associated with the given entity.
     *
     * @param entity_id Id of the entity to associate the component with.
     */
    auto emplace_default(std::size_t entity_id) noexcept ->
        std::enable_if_t<std::is_default_constructible_v<T>, void> {
        if (!is_valid_entity_id(entity_id)) {
            error_not_enough_capacity(
                "Sparse vector too small",
                entity_id + 1,
                sparse_.size()
            ); return;
        }

        std::cout << "Emplacing new component for entity " << entity_id << std::endl;

        if (contains(entity_id)) {
            error_entity_already_has_component(entity_id);
            return;
        }

        dense_.emplace_back();
        std::size_t component_index = dense_.size() - 1;

        sparse_[entity_id] = component_index;
        reverse_.push_back(entity_id);
    }

    /**
     * @brief Emplaces a type T component in the sparse set
     *        associated with multiple entities into the sparse set.
     *
     * @param entity_ids Ids of all the entities to associate the component with.
     * 
     * @note All entity_ids must be unsigned integers.
     * @note Requires at least one entity_id.
     */
    template<typename... Ts>
    auto batch_emplace(Ts&&... entity_ids) noexcept ->
        std::enable_if_t<std::is_default_constructible_v<T>
        && (std::is_integral_v<Ts> && ...) && (std::is_unsigned_v<Ts> && ...)
        && (sizeof...(Ts) > 0) ,void> {
            (emplace_default(std::forward<Ts>(entity_ids)), ...);
    }

    /**
     * @brief Removes an entity from the sparse set using swap-and-pop strategy.
     * 
     * @param entity_ids Id of the entity to associate the component with.
     */
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

    /**
     * @brief Removes an entity from the sparse set using swap-and-pop strategy.
     *
     * @param entity_ids Ids of all the entities to associate the component with.
     *
     * @note All entity_ids must be unsigned integers.
     * @note Requires at least one entity_id.
     */
    template<typename... Ts>
    auto batch_remove_swap(Ts&&... entity_ids) noexcept ->
        std::enable_if_t<(std::is_integral_v<Ts> && ...)
        && (std::is_unsigned_v<Ts> && ...) && (sizeof...(Ts) > 0), void> {
            (remove_swap(std::forward<Ts>(entity_ids)), ...);
    }

public:
    bool contains(std::size_t entity_id) const {
        return is_valid_entity_id(entity_id) && sparse_[entity_id] != SIZE_MAX;
    }

    template<typename... Ts>
    auto batch_contains(Ts&&... entity_ids) const noexcept ->
        std::enable_if_t<(std::is_integral_v<Ts> && ...)
        && (std::is_unsigned_v<Ts> && ...) && (sizeof...(Ts) > 0), bool> {
            return (contains(std::forward<Ts>(entity_ids)) && ...);
    }

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
    T* operator[](std::size_t entity_id) noexcept { // NOTE: unsafe
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return nullptr;
        }
        return &dense_[sparse_[entity_id]];
    }

    const T* operator[](std::size_t entity_id) const noexcept { // NOTE: unsafe
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return nullptr;
        }
        return &dense_[sparse_[entity_id]];
    }

    T* get(std::size_t entity_id) & noexcept { // NOTE: unsafe
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return nullptr;
        }
        return &dense_[sparse_[entity_id]];
    }

    const T* get(std::size_t entity_id) const& noexcept { // NOTE: unsafe
        if (!is_valid_entity_id(entity_id) || sparse_[entity_id] == SIZE_MAX) {
            error_not_enough_capacity(
                "Sparse vector too small or entity has no component",
                entity_id + 1,
                sparse_.size()
            );
            return nullptr;
        }
        return &dense_[sparse_[entity_id]];
    }

private:
    std::vector<std::size_t> sparse_; // contient un entity_id -> component_id
    std::vector<std::size_t> reverse_; // contient un composante_id -> entity_id
    std::vector<T> dense_;  //contient un component_id -> component (T)
};

class RegistryBuilder {
public:
    void WithA() const noexcept {
        std::cout << "You are now in the Registry Builder" << std::endl;;
    }

    [[nodiscard]] static RegistryBuilder& initialize() noexcept {
        static RegistryBuilder builder;
        return builder;
    }

private:
    
};

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
      * 
      * @note The component must be registered in the registry.
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
     * @note The component must be registered in the registry.
     * @note All entity_ids must be unsigned integers.
     * @note Passing zero IDs is not allowed.
     */
    template<typename T, typename... Ids>
    auto emplace_all(Ids&&... entity_ids) noexcept ->
        std::enable_if_t<std::is_base_of_v<Component<T>, T>
        && ((std::is_integral_v<Ids> && std::is_unsigned_v<Ids>) && ...)
        && (sizeof...(Ids) > 0), void> {
            std::get<Sparse<T>>(storage_) //faire fonction get interne
                .batch_emplace(std::forward<Ids>(entity_ids)...);
    }

    /**
     * @brief Retrieves the component of type T associated with the specified entity ID.
     *
     * @tparam T The component type (must inherit from Component<T>
     *           and be part of the Registry type list).
     * @param entity_id ID of the entity whose component is being required.
     *
     * @return A reference to the component of type T.
     * 
     * @note The component must be registered in the registry.
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
     * 
     * @note The component must be registered in the registry.
     * @note All entity_ids must be unsigned integers.
     * @note Passing zero IDs is not allowed.
     */
    template<typename T, typename... Ids>
    [[nodiscard]] auto get_all(Ids&&... entity_ids) noexcept ->
        std::enable_if_t<std::is_base_of_v<Component<T>, T>
        && ((std::is_integral_v<Ids> && std::is_unsigned_v<Ids>) && ...)
        && std::disjunction_v<std::is_same<T, Ts>...>
        && (sizeof...(Ids) > 0), std::tuple<T&>> {
            return std::tie(get<T>(entity_ids)...);
    }

    /**
     * @brief Removes the component of type T associated with the given entity ID.
     * 
     * @tparam T The component type (must inherit from Component<T>).
     * @param entity_id ID of the entity whose component is being required.
     *
     * @note The component must be registered in the registry.
     */
    template<typename T>
    auto remove(std::size_t entity_id) noexcept ->
        std::enable_if_t<std::is_base_of_v<Component<T>, T>
        && std::disjunction_v<std::is_same<T, Ts>...>, void> {
            std::get<Sparse<T>>(storage_).remove_swap(entity_id);
    }

    /**
     * @brief Removes the component of type T associated with all given entitiy IDs.
     *
     * @tparam T The component type (must inherit from Component<T>).
     * @param entity_ids Variadic pack of entity IDs to which the component is added.
     *
     * @note The component must be registered in the registry.
     * @note All entity_ids must be unsigned integers.
     * @note Passing zero IDs is not allowed.
     */
    template<typename T, typename... Ids>
    auto remove_all(Ids&&... entity_ids) noexcept ->
        std::enable_if_t<std::is_base_of_v<Component<T>, T>
        && std::disjunction_v<std::is_same<T, Ts>...>
        && ((std::is_integral_v<Ids> && std::is_unsigned_v<Ids>) && ...)
        && (sizeof...(Ids) > 0), void> {
            std::get<Sparse<T>>(storage_)
                .batch_remove_swap(std::forward<Ids>(entity_ids)...);
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

//using Registry = Registry<int, float>;

template<typename... Ts>
class oldRegistry {
public:
    oldRegistry(std::size_t maxEntities, std::size_t maxPool) {
        /* _entityToPosIndex.resize(maxEntities, -1);
         _entityToVelIndex.resize(maxEntities, -1);
         _entityToRotIndex.resize(maxEntities, -1);
         _entityToScaleIndex.resize(maxEntities, -1);
         _entityToColorIndex.resize(maxEntities, -1);

         _positions.reserve(maxPool);
         _velocities.reserve(maxPool);
         _rotations.reserve(maxPool);
         _scales.reserve(maxPool);
         _colors.reserve(maxPool);*/
    }

public:
    template<typename T>
    auto add(std::size_t entity_id) noexcept ->
    /* @note Check if the components if already present in the storage_,
    otherwise emplace it. */

    std::enable_if_t<std::is_base_of_v<Component<T>, T>, void> {
        std::cout << "[Not implemented] Adding "
            << typeid(std::type_identity_t<T>).name()
            << " to entity "
            << entity_id
            << std::endl;
    }

    template<typename T, typename... Ts>
    auto add_batch(Ts&&... entity_ids) noexcept ->
    std::enable_if_t<std::is_base_of_v<Component<T>, T>
    && (sizeof...(entity_ids) > 0), void> {
        std::cout << "[Not implemented] Adding "
            << typeid(std::type_identity_t<T>).name()
            << " to entity ";
            (... ,(std::cout << entity_ids << " "))
            << std::endl;
    }

public:
    template<typename T>
    auto get(std::size_t entity_id) const noexcept ->
    std::enable_if_t<std::is_base_of_v<Component<T>, T>, T&> {
        std::cout << "[Not implemented] Getting "
                  << typeid(std::type_identity_t<T>).name()
                  << " to entity "
                  << entity_id
                  << std::endl;

        static T dummy{};
        return dummy;
    }

public:
    template<typename T, bool Debug = true>
    auto get_sparse() noexcept ->
    std::enable_if_t<(std::is_base_of_v<Component<T>, T>
    && (std::is_same_v<T, Ts> || ...)), Sparse<T>& > {
        if constexpr (Debug)
            return std::get<Sparse<T>>(reg_);
    }

    template<bool Debug = true>
    void print_all_types() const noexcept {
        if constexpr (Debug)
            std::apply([&](const auto&... args) {
                ((std::cout << typeid(args).name() << " "), ...);
                std::cout << std::endl;
            }, reg_);
    }

public:
    /**
     * @brief Checks if the storage_ contains the given T sparse set.
     * @tparam T Type that inherits from ISparseSet.
     * @return True if the storage_ contains the T sparse set, false otherwise.
     */
    template<typename T>
    [[nodiscard]] auto has_component() const noexcept
        -> std::enable_if_t<std::is_base_of_v<ISparseSet, T>, bool> {
        return storage_.contains(std::type_index(typeid(T)));
    }

    /**
     * @brief Returns a reference to the SparseSet of type T if it exists in the storage.
     * @tparam T Type that inherits from ISparseSet.
     * @return Pointer containing a reference to the SparseSet if found,
     *         or nullptr otherwise.
     */
    template<typename T>
    [[nodiscard]] auto get_sparse_by_type() noexcept
        -> std::enable_if_t<std::is_base_of_v<ISparseSet, T>, const Sparse<T>*> {
        if (this->has_component<T>()) {
            auto it = storage_.find(std::type_index(typeid(T)));
            if (it != storage_.end()) return static_cast<const Sparse<T>*>(it->second.get());
        }
        return nullptr;
    }

    /**
     * @brief Returns a reference to the SparseSet of type T if it exists in the storage.
     * @tparam T Type of the component contained in the sparse set.
     * @return Pointer containing a reference to the SparseSet if found,
     *         or nullptr otherwise.
     */
    template<typename T>
    [[nodiscard]] const Sparse<T>* get_sparse_for_component() noexcept {
        if (this->has_component<Sparse<T>>()) {
            auto it = storage_.find(std::type_index(typeid(T)));
            if (it != storage_.end()) return static_cast<const Sparse<T>*>(it->second.get());
        }
        return nullptr;
    }

public: /* DEBUG FONCTIONS */
    template<typename T, bool Debug = true>
    auto emplace_sparse_in_storage_() noexcept ->
    std::enable_if_t<std::is_base_of_v<ISparseSet, T>
    && std::is_default_constructible_v<T>, void> {
        /* @note Mock implementation only */
        if constexpr (Debug)
            storage_[typeid(T)] = std::make_unique<T>(100);
    }

private:
    std::tuple<Sparse<Ts>...> reg_; 
    //FINALEMENT GARDER LE TUPLE, C'EST PLUS PERF
    //L'AUTRE EST PLUS FLEXIBLE MAIS MOINS BON

    /* @note N'oublie par de réserver la capacité lors du constructeur */
    std::unordered_map<std::type_index, std::unique_ptr<ISparseSet>> storage_;
};

int main() {
    std::cout << "Hello World!\n";

    constexpr size_t entityCount = 100000000;
    constexpr size_t maxPool = 200000;

    {
        oldRegistry<Position> registry(entityCount, maxPool);

        registry.add<Position>(0);
        registry.add_batch<Position>(0, 1, 2, 3, 4, 5);

        registry.print_all_types();

        auto comp = registry.get<DummyComponent>(0);


        auto sparse = registry.get_sparse<Position>();

        sparse.insert(0, Position{});


        registry.emplace_sparse_in_storage_<Sparse<int>>();
        registry.emplace_sparse_in_storage_<Sparse<float>>();
    }
   
    system("cls");

    {
        //Registry<int, float, double, bool> world;
    

        //Registry<int, double, char>& r; // juste pour montrer les types, on n’en a pas vraiment besoin
        
        std::size_t e0 = 0;
        std::size_t e1 = 1;
        std::size_t e2 = 2;
        std::size_t e3 = 3;
        std::size_t e4 = 4;
        std::size_t e5 = 5;
        
        Registry<Position, Velocity> registry;

        registry.emplace<Position>(e0);
        registry.emplace<Velocity>(e0);
        registry.emplace<Velocity>(e0);
        registry.emplace<Velocity>(e0);

        registry.emplace_all<Velocity>(std::move(e0), std::move(e1),
            std::move(e2), std::move(e3), std::move(e4), std::move(e5));

        auto vel = registry.get<Velocity>(0);

        vel.test();

        //movement_system(registry.view<Position, Velocity>()


        //Registry[
        //   
        //]

        //registry.create();

        //Registry::initialize()
        // .with
    }



    //Sparse<int> sparse;

    //sparse.insert(0, 10);
}
```
