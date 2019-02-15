#include "Masternode.h"
#include "Router.h"

using namespace masternode;

Masternode::Masternode(std::shared_ptr<MasternodeConfig> config) {
    config_ = config;
    state_ = std::make_shared<NetworkState>(config_);
    cache_ = std::make_shared<ContentCache>(256, config_->cache_directory);
    sw_ = std::make_shared<ServiceWorker>(config_->service_worker_path);
    config_->options.handlerFactories = proxygen::RequestHandlerChain()
        .addThen<Router>(config_, state_, cache_, sw_)
        .build();

    server_ = std::make_unique<proxygen::HTTPServer>(
        std::move(config_->options));
}

void Masternode::start(std::function<void()> onSuccess,
    std::function<void(std::exception_ptr)> onError) {
    if (state_) {
        state_->beginPollingGateway();
    }
    server_->bind(config_->IPs);
    server_->start(onSuccess, onError);
}

void Masternode::stop() {
    if (server_.get()) {
        server_->stop();
    }
}

std::shared_ptr<NetworkState> Masternode::getNetworkState() { return state_; }