#include <fwdv0/que.h>
using fwdv0::Que;
#include <doctest/doctest.h>
#include <rapidcheck.h>

#include <deque>
#include <vector>
#include <algorithm>

using namespace std::string_literals;

TEST_CASE("Default-constructable and copyable/movable") {
  Que<int> empty;
  //Que<int> empty2 {empty};
  //Que<int> empty3 {std::move(empty2)};
  //empty2 = empty;
  //empty3 = std::move(empty);
}

using TypeRef = std::deque<long>;
using TypeHyp = Que<std::deque<long>>;
using PairRef = std::pair<TypeHyp, TypeRef>;
using InputData = std::vector<long>;

std::vector<std::pair<const char*, PairRef (*)(InputData)>> aDataMaker {
  {"emplace_back", [](InputData data) -> PairRef
    {
      TypeHyp hyp;
      TypeRef ref;
      RC_ASSERT(hyp.size() == 0);
      for (const auto& val : data) {
        auto& obj = hyp.emplace_back();
        obj.emplace_back(val);
        ref.emplace_back(val);
      }
      return {hyp, ref};
    }},
  {"emplace_front", [](InputData data) -> PairRef
    {
      TypeHyp hyp;
      TypeRef ref;
      RC_ASSERT(hyp.size() == 0);
      for (const auto& val : data) {
        auto& obj = hyp.emplace_front();
        obj.emplace_front(val);
        ref.emplace_front(val);
      }
      return {hyp, ref};
    }},
  {"emplace_mixed", [](InputData data) -> PairRef
    {
      TypeHyp hyp;
      TypeRef ref;
      for (const auto& val : data) {
        if (val%2==0) {
          auto& obj = hyp.emplace_front();
          obj.emplace_back(val);
          ref.push_front(val);
        } else {
          auto& obj2 = hyp.emplace_back();
          obj2.emplace_back(val);
          ref.push_back(val);
        }
      }
      return {hyp, ref};
    }},
};

std::vector<std::pair<const char*, void (*)(TypeHyp&, TypeRef&)>> aDataChecker {
  {"random access", [](TypeHyp& hyp, TypeRef& ref)
    {
      RC_ASSERT(hyp.size() == ref.size());
      for (auto i=0; i<ref.size(); i++) {
        RC_ASSERT(hyp.at(i).at(0) == ref.at(i));
        RC_ASSERT(hyp[i][0] == ref[i]);
      }
    }},
  {"const random access", [](TypeHyp& hyp, TypeRef& ref)
    {
      const TypeHyp& chyp = hyp;
      RC_ASSERT(chyp.size() == ref.size());
      for (auto i=0; i<ref.size(); i++) {
        RC_ASSERT(chyp.at(i).at(0) == ref.at(i));
        RC_ASSERT(chyp[i][0] == ref[i]);
      }
    }},
  {"iterator", [](TypeHyp& hyp, TypeRef& ref)
    {
      auto itr {hyp.begin()}, itrEnd {hyp.end()};
      auto itrRef {ref.begin()}, itrRefEnd {ref.end()};
      for (; itrRef != itrRefEnd; ++itr, ++itrRef) {
        RC_ASSERT(itr->at(0) == *itrRef);
        RC_ASSERT((*itr).at(0) == *itrRef);
        RC_ASSERT(itr != itrEnd);
      }
      RC_ASSERT(itr == itrEnd);
    }},
  {"iterator-from-const", [](TypeHyp& hyp, TypeRef& ref)
    {
      const TypeHyp& chyp = hyp;
      auto itr {chyp.begin()}, itrEnd {chyp.end()};
      auto itrRef {ref.begin()}, itrRefEnd {ref.end()};
      for (; itrRef != itrRefEnd; ++itr, ++itrRef) {
        RC_ASSERT(itr->at(0) == *itrRef);
        RC_ASSERT((*itr).at(0) == *itrRef);
        RC_ASSERT(itr != itrEnd);
      }
      RC_ASSERT(itr == itrEnd);
    }},
  {"const iterator", [](TypeHyp& hyp, TypeRef& ref)
    {
      auto itr {hyp.cbegin()}, itrEnd {hyp.cend()};
      auto itrRef {ref.cbegin()}, itrRefEnd {ref.cend()};
      for (; itrRef != itrRefEnd; ++itr, ++itrRef) {
        RC_ASSERT(itr->at(0) == *itrRef);
        RC_ASSERT((*itr).at(0) == *itrRef);
        RC_ASSERT(itr != itrEnd);
      }
      RC_ASSERT(itr == itrEnd);
    }},
};

TEST_CASE("Read/Write test") {
  for (const auto& [descW, W] : aDataMaker) {
    for (const auto& [descC, C] : aDataChecker) {
      CHECK(rc::check("W("s + descW + ") C(" + descC + ")", [&](InputData data) {
            auto [hyp, ref] = W(data);
            C(hyp, ref);
            }));
    }
  }
}

