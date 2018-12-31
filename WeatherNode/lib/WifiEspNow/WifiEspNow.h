#ifndef WIFIESPNOW_H
#define WIFIESPNOW_H

#include <cstddef>
#include <cstdint>

#if defined(ESP8266)
#include <c_types.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#else
#error "This library supports ESP8266 and ESP32 only."
#endif

/** \brief Key length.
 */
static const int WIFIESPNOW_KEYLEN = 16;

/** \brief Maximum message length.
 */
static const int WIFIESPNOW_MAXMSGLEN = 250;

struct WifiEspNowPeerInfo {
  uint8_t mac[6];
  uint8_t channel;
};

/** \brief Result of send operation.
 */
enum class WifiEspNowSendStatus : uint8_t {
  NONE = 0, ///< result unknonw, send in progress
  OK   = 1, ///< sent successfully
  FAIL = 2, ///< sending failed
};

class WifiEspNowClass
{
public:
  WifiEspNowClass();

  /** \brief Initialize ESP-NOW.
   *  \return whether success
   */
 bool
  #if defined(ESP8266)
  begin(esp_now_role role = ESP_NOW_ROLE_COMBO);
  #elif defined(ESP32)
  begin();
  #endif

  /** \brief Stop ESP-NOW.
   */
  void
  end();

  /** \brief List current peers.
   *  \param[out] peers buffer for peer information
   *  \param maxPeers buffer size
   *  \return total number of peers, \p std::min(retval,maxPeers) is written to \p peers
   */
  int
  listPeers(WifiEspNowPeerInfo* peers, int maxPeers) const;

  /** \brief Test whether peer exists.
   *  \param mac peer MAC address
   *  \return whether peer exists
   */
  bool
  hasPeer(const uint8_t mac[6]) const;

  /** \brief Add a peer or change peer channel.
   *  \param mac peer MAC address
   *  \param channel peer channel, 0 for current channel
   *  \param key encryption key, nullptr to disable encryption
   *  \return whether success
   *  \note To change peer key, remove the peer and re-add.
   */
  bool
  #if defined(ESP8266)
  addPeer(const uint8_t mac[6], int channel = 0, esp_now_role role = ESP_NOW_ROLE_COMBO, const uint8_t key[WIFIESPNOW_KEYLEN] = nullptr);
  #elif defined(ESP32)
  addPeer(const uint8_t mac[6], int channel = 0, wifi_interface_t ifidx = ESP_IF_WIFI_STA, const uint8_t key[WIFIESPNOW_KEYLEN] = nullptr);
  #endif
  /** \brief Remove a peer.
   *  \param mac peer MAC address
   *  \return whether success
   */
  bool
  removePeer(const uint8_t mac[6]);

  typedef void (*RxCallback)(const uint8_t mac[6], const uint8_t* buf, size_t count, void* cbarg);

  /** \brief Set receive callback.
   *  \param cb the callback
   *  \param cbarg an arbitrary argument passed to the callback
   *  \note Only one callback is allowed; this replaces any previous callback.
   */
  void
  onReceive(RxCallback cb, void* cbarg);

  /** \brief Send a message.
   *  \param mac destination MAC address, nullptr for all peers
   *  \param buf payload
   *  \param count payload size, must not exceed \p WIFIESPNOW_MAXMSGLEN
   *  \return whether success (message queued for transmission)
   */
  bool
  send(const uint8_t mac[6], const uint8_t* buf, size_t count);

  /** \brief Retrieve status of last sent message.
   *  \return whether success (unicast message received by peer, multicast message sent)
   */
  WifiEspNowSendStatus
  getSendStatus() const
  {
    return m_txRes;
  }

private:
  static void
  rx(const uint8_t* mac, const uint8_t* data, uint8_t len);

  static void
  tx(const uint8_t* mac, uint8_t status);

private:
  RxCallback m_rxCb;
  void* m_rxCbArg;
  WifiEspNowSendStatus m_txRes;
};

extern WifiEspNowClass WifiEspNow;

#endif // WIFIESPNOW_H
