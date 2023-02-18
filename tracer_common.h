//
// Created by curtis on 2/18/23.
//

#pragma once

#include <opentelemetry/exporters/otlp/otlp_http_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_context.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/trace/samplers/parent.h>
#include <opentelemetry/trace/provider.h>

#include <opentelemetry/sdk/resource/resource.h>

#include <opentelemetry/context/propagation/global_propagator.h>
#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/trace/propagation/http_trace_context.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <vector>


namespace otlp = opentelemetry::exporter::otlp;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace nostd = opentelemetry::nostd;

namespace {
template<typename T>
class HttpTextMapCarrier: public opentelemetry::context::propagation::TextMapCarrier {
public:
    HttpTextMapCarrier<T>(T &headers) : headers_(headers) {}
    HttpTextMapCarrier() = default;
    virtual nostd::string_view Get(
        nostd::string_view key) const noexcept override
    {
        std::string key_to_compare = key.data();
        if (key == opentelemetry::trace::propagation::kTraceParent) {
            key_to_compare = "Traceparent";
        } else if (key == opentelemetry::trace::propagation::kTraceState) {
            key_to_compare = "Tracestate";
        }

        auto iter = headers_.find(key_to_compare);
        if (iter != headers_.end()) {
            return iter->second;
        }
        return "";
    }

    virtual void Set(nostd::string_view key,
                     nostd::string_view value) noexcept override {
        headers_.insert(
                std::pair<std::string, std::string>(std::string(key), std::string(value))
                );
    }
    T headers_;
};

otlp::OtlpHttpExporterOptions opts;

void InitTracer() {
    auto exporter =
        std::unique_ptr<trace_sdk::SpanExporter>(new otlp::OtlpHttpExporter(opts));
    auto processor =
        std::unique_ptr<trace_sdk::SpanProcessor>(
            new trace_sdk::SimpleSpanProcessor(std::move(exporter)));
    std::vector<std::unique_ptr<trace_sdk::SpanProcessor>> processors;
    processors.push_back(std::move(processor));

    auto resource_attrs = opentelemetry::sdk::resource::ResourceAttributes {
        {"service.name", "fib-times2"}
    };
    auto resource = opentelemetry::sdk::resource::Resource::Create(resource_attrs);
    auto sampler = std::unique_ptr<trace_sdk::AlwaysOnSampler>
        (new trace_sdk::AlwaysOnSampler);

    auto context =
        std::make_shared<trace_sdk::TracerContext>(
            std::move(processors), resource, std::move(sampler));

    auto provider = nostd::shared_ptr<opentelemetry::trace::TracerProvider>
            (new trace_sdk::TracerProvider(context));
    opentelemetry::trace::Provider::SetTracerProvider(provider);

    opentelemetry::context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(
        opentelemetry::nostd::shared_ptr<opentelemetry::context::propagation::TextMapPropagator>(
            new opentelemetry::trace::propagation::HttpTraceContext()));
}

void CleanUpTracer() {
    std::shared_ptr<opentelemetry::trace::TracerProvider> none;
    opentelemetry::trace::Provider::SetTracerProvider(none);
}

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> get_tracer(std::string tracer_name) {
    auto provider = opentelemetry::trace::Provider::GetTracerProvider();
    return provider->GetTracer(tracer_name);
}
}

