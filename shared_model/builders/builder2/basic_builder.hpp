#ifndef IROHA_BASIC_BUILDER_HPP
#define IROHA_BASIC_BUILDER_HPP

#include <tuple>

template <typename BackendBuilder, typename BuildPolicy, int S>
struct BuilderProperties {
  BackendBuilder b;
  BuildPolicy bp;
  constexpr static int s{S};
};

template <template <typename, typename, int> class Builder,
          typename BackendBuilder,
          typename BuildPolicy,
          int S>
constexpr BuilderProperties<BackendBuilder, BuildPolicy, S>
extractBuilderProperties(Builder<BackendBuilder, BuildPolicy, S> &&);

template <typename BP>
constexpr int getS(const BP &bp) {
  return bp.s;
}

template <typename Builder>
using BackendBuilder =
    decltype(extractBuilderProperties(std::declval<Builder>()).b);

template <typename Builder>
using BuildPolicy =
    decltype(extractBuilderProperties(std::declval<Builder>()).bp);

template <typename Builder>
constexpr int S =
    decltype(extractBuilderProperties(std::declval<Builder>()))::s;

enum class SetterPolicy { Copy, Move };

// TODO: use ExtractBuilderPolicy
template <template <typename, typename, int> class Builder,
          typename BackendBuilder,
          typename BuildPolicy,
          int S,
          int s>
Builder<BackendBuilder, BuildPolicy, s> nextBuilder(
    Builder<BackendBuilder, BuildPolicy, S> &&);

/**
 * returns new type with all the same parameters except S,
 * which is set to a new value
 */
template <typename Builder, int S>
using NewBuilderType = typename std::decay_t<Builder>::template NextBuilder<S>;

// template <typename Builder, int s>
// using NewBuilderType = decltype(
//    nextBuilder<BackendBuilder<Builder>, BuildPolicy<Builder>, S<Builder>,
//    s>(std::declval<std::decay_t<Builder>>()));

/**
 * returns new type with all the same parameters except S,
 * which is modified on the offset specified by Fields
 */
template <typename Builder, int Fields>
using NextBuilderType =
    NewBuilderType<Builder, std::decay_t<Builder>::kS | (1 << Fields)>;

/**
 * @return SetterPolicy from a builder
 */
template <typename Builder>
constexpr SetterPolicy getSetterPolicy() {
  return std::decay_t<Builder>::kSetterPolicy;
}

/**
 * Creates NewBuilder object by copying b
 */
template <typename NewBuilder, typename Builder>
auto makeNewBuilder(Builder &&b)
    -> std::enable_if_t<getSetterPolicy<Builder>() == SetterPolicy::Copy,
                        NewBuilder> {
  return NewBuilder(b);
}

/**
 * Creates NewBuilder object by moving b
 */
template <typename NewBuilder, typename Builder>
auto makeNewBuilder(Builder &&b)
    -> std::enable_if_t<getSetterPolicy<Builder>() == SetterPolicy::Move,
                        NewBuilder> {
  return NewBuilder(std::move(b));
}

/**
 * Copies builder
 */
template <typename Builder>
auto makeBuilder(Builder &&b)
    -> std::enable_if_t<getSetterPolicy<Builder>() == SetterPolicy::Copy,
                        std::decay_t<Builder>> {
  return Builder(b);
}

/**
 * Moves builder
 */
template <typename Builder>
auto makeBuilder(Builder &&b)
    -> std::enable_if_t<getSetterPolicy<Builder>() == SetterPolicy::Move,
                        std::decay_t<Builder>> {
  return Builder(std::move(b));
}

/**
 * Creates new builder with Fields field considered to be set.
 * Copies value from old builder
 */
template <typename Builder, int Fields>
auto makeNextBuilder(Builder &&b)
    -> std::enable_if_t<getSetterPolicy<Builder>() == SetterPolicy::Copy,
                        NextBuilderType<Builder, Fields>> {
  return NextBuilderType<Builder, Fields>(b);
}

/**
 * Creates new builder with Fields field considered to be set.
 * Moves value from old builder
 */
template <typename Builder, int Fields>
auto makeNextBuilder(Builder &&b)
    -> std::enable_if_t<getSetterPolicy<Builder>() == SetterPolicy::Move,
                        NextBuilderType<Builder, Fields>> {
  return NextBuilderType<Builder, Fields>(std::move(b));
}

/**
 * Applies transformation to the builder
 * @tparam Fields - field which is considered to be set in a result
 * @tparam Transformation - function which accepts transport builder
 * and modifies it.
 * @param b - Derived from BasicBuilder class
 * @return New builder with applied transformation
 */
template <int Fields, typename Builder, typename Transformation>
auto transform(Builder &&b, Transformation &&t) {
  auto copy = makeNextBuilder<Builder, Fields>(std::forward<Builder>(b));
  t(copy.backend_builder_);
  return copy;
}

/**
 * Base class for all builders which create interface objects.
 */
template <typename BuilderImpl>
class BasicBuilder {
 private:
  using BB = BackendBuilder<BuilderImpl>;
  using BP = BuildPolicy<BuilderImpl>;

 public:
  using Impl = BuilderImpl;
  /**
   * Construct object from internal state of the builder
   * @return Return type is the return type of the BuildPolicy
   */
  auto build() {
    static_assert(kS == (1 << BuilderImpl::TOTAL) - 1,
                  "Required fields are not set");
    return build_func_(backend_builder_.build());
  }

  BasicBuilder() = default;

  template <typename Builder>
  BasicBuilder(Builder &&o)
      : backend_builder_(o.backend_builder_), build_func_(o.build_func_) {}

  /**
   * Setter which uses concrete implementation from BackendBuilder to set
   * state of the object.
   */
  auto fromImplementation(const typename BB::ImplType &impl) {
    auto copy = makeNewBuilder<
        NewBuilderType<BuilderImpl, (1 << BuilderImpl::TOTAL) - 1>>(*this);
    copy.backend_builder_ = copy.backend_builder_.fromImplementation(impl);
    return copy;
  }

  BB backend_builder_;
  BP build_func_;

  static constexpr enum SetterPolicy kSetterPolicy = BB::kSetterPolicy;
  static constexpr int kS = S<BuilderImpl>;
};

#endif  // IROHA_BASIC_BUILDER_HPP
