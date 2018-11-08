
#include <test.hpp>

#include <vtzero/layer.hpp>
#include <vtzero/vector_tile.hpp>

#include <cstddef>

TEST_CASE("default constructed layer") {
    vtzero::layer layer{};
    REQUIRE_FALSE(layer.valid());
    REQUIRE_FALSE(layer);

    REQUIRE(layer.data() == vtzero::data_view{});
    REQUIRE_ASSERT(layer.version());
    REQUIRE_ASSERT(layer.get_tile());
    REQUIRE_ASSERT(layer.extent());
    REQUIRE_ASSERT(layer.name());

    REQUIRE(layer.empty());
    REQUIRE(layer.num_features() == 0);

    REQUIRE_THROWS_AS(layer.key_table(), const assert_error&);
    REQUIRE_THROWS_AS(layer.value_table(), const assert_error&);

    REQUIRE_THROWS_AS(layer.key(0), const assert_error&);
    REQUIRE_THROWS_AS(layer.value(0), const assert_error&);

    REQUIRE_THROWS_AS(layer.get_feature_by_id(0), const assert_error&);
    REQUIRE_ASSERT(layer.begin());
    REQUIRE_ASSERT(layer.end());
}

TEST_CASE("read a layer") {
    const auto data = load_test_tile();
    const vtzero::vector_tile tile{data};

    auto layer = tile.get_layer_by_name("bridge");
    REQUIRE(layer.valid());
    REQUIRE(layer);

    REQUIRE(layer.version() == 1);
    REQUIRE_FALSE(layer.get_tile().valid());
    REQUIRE(layer.extent() == 4096);
    REQUIRE(layer.name() == "bridge");

    REQUIRE_FALSE(layer.empty());
    REQUIRE(layer.num_features() == 2);

    const auto& kt = layer.key_table();
    REQUIRE(kt.size() == 4);
    REQUIRE(kt[0] == "class");

    const auto& vt = layer.value_table();
    REQUIRE(vt.size() == 4);
    REQUIRE(vt[0].type() == vtzero::property_value_type::string_value);
    REQUIRE(vt[0].string_value() == "main");
    REQUIRE(vt[1].type() == vtzero::property_value_type::int_value);
    REQUIRE(vt[1].int_value() == 0);

    REQUIRE(layer.key(0) == "class");
    REQUIRE(layer.key(1) == "oneway");
    REQUIRE(layer.key(2) == "osm_id");
    REQUIRE(layer.key(3) == "type");
    REQUIRE_THROWS_AS(layer.key(4), const vtzero::out_of_range_exception&);

    REQUIRE(layer.value(0).string_value() == "main");
    REQUIRE(layer.value(1).int_value() == 0);
    REQUIRE(layer.value(2).string_value() == "primary");
    REQUIRE(layer.value(3).string_value() == "tertiary");
    REQUIRE_THROWS_AS(layer.value(4), const vtzero::out_of_range_exception&);
}

TEST_CASE("access features in a layer by id") {
    const auto data = load_test_tile();
    const vtzero::vector_tile tile{data};

    auto layer = tile.get_layer_by_name("building");
    REQUIRE(layer);

    REQUIRE(layer.num_features() == 937);

    const auto feature = layer.get_feature_by_id(122);
    REQUIRE(feature.integer_id() == 122);
    REQUIRE_FALSE(feature.has_attributes());
    REQUIRE(feature.geometry_type() == vtzero::GeomType::POLYGON);
    REQUIRE_FALSE(feature.geometry_data().empty());

    REQUIRE_FALSE(layer.get_feature_by_id(844));
    REQUIRE_FALSE(layer.get_feature_by_id(999999));
}

TEST_CASE("iterate over all features in a layer") {
    const auto data = load_test_tile();
    const vtzero::vector_tile tile{data};

    auto layer = tile.get_layer_by_name("building");
    REQUIRE(layer);

    std::size_t count = 0;

    for (auto it = layer.begin(); it != layer.end(); ++it) {
        ++count;
    }

    REQUIRE(count == 937);
}

TEST_CASE("iterate over some features in a layer") {
    const auto data = load_test_tile();
    const vtzero::vector_tile tile{data};

    const auto layer = tile.get_layer_by_name("building");
    REQUIRE(layer);

    uint64_t id_sum = 0;

    for (const auto feature : layer) {
        if (feature.integer_id() == 10) {
            break;
        }
        id_sum += feature.integer_id();
    }

    const uint64_t expected = (10 - 1) * 10 / 2;
    REQUIRE(id_sum == expected);

    const auto feature = *layer.begin();
    REQUIRE(feature);
    REQUIRE(feature.integer_id() == 1);
}

