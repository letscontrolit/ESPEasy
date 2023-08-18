// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#pragma once

ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE

template <typename TString, typename Enable = void>
struct StringAdapter;

template <typename TString, typename Enable = void>
struct SizedStringAdapter;

template <typename TString>
typename StringAdapter<TString>::AdaptedString adaptString(const TString& s) {
  return StringAdapter<TString>::adapt(s);
}

template <typename TChar>
typename StringAdapter<TChar*>::AdaptedString adaptString(TChar* p) {
  return StringAdapter<TChar*>::adapt(p);
}

template <typename TChar>
typename SizedStringAdapter<TChar*>::AdaptedString adaptString(TChar* p,
                                                               size_t n) {
  return SizedStringAdapter<TChar*>::adapt(p, n);
}

ARDUINOJSON_END_PRIVATE_NAMESPACE
