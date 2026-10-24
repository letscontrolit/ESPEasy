#pragma once

#include "../../_Plugin_Helper.h"
#ifdef USES_P190

# include <algorithm>
# include <deque>
# include <vector>
# include <ESPeasySerial.h>

# include "P190_protocol.h"

# define P190_BAUD_MULT        PCONFIG(0)       // Baud rate / 9600: 1, 2, 4, 6 or 12
# define P190_LED_FEEDBACK     PCONFIG(1)       // 1 = show match / no match / enrolment on the LED ring
# define P190_MATCH_COLOR      PCONFIG(2)       // LED color of a match (P190_proto::COLOR_...)
# define P190_NOMATCH_COLOR    PCONFIG(3)       // LED color of no match
# define P190_POLL_MS          PCONFIG(4)       // Scan interval when no touch GPIO is set (ms)
# define P190_MIN_SCORE        PCONFIG(5)       // Matches with a lower score are reported as no match
# define P190_PASSWORD         PCONFIG_ULONG(0) // Module password (default 0)
# define P190_ADDRESS          PCONFIG_ULONG(1) // Module address (default 0xFFFFFFFF)

# define P190_DEF_BAUD_MULT    6                // 57600 baud
# define P190_DEF_POLL_MS      250
# define P190_DEF_CAPACITY     200              // R503; replaced by the library size read from the module
# define P190_MAX_CAPACITY     3000             // Largest library size accepted from the module
# define P190_ACK_TIMEOUT_MS   2000             // Max wait for an acknowledge
# define P190_SEARCH_TIMEOUT_MS 3000            // Max wait for a Search acknowledge: it goes through the whole library

# define P190_VALUE_ID         0
# define P190_VALUE_SCORE      1
# define P190_VALUE_COUNT      2
# define P190_VALUE_ENROLL     3

// Value Enroll: progress of the last enrolment, for pages and controllers that don't get the rules events
# define P190_ENROLL_NONE      0      // No enrolment yet, or accepted and not started
# define P190_ENROLL_PLACE1    1      // Place the finger (capture 1)
# define P190_ENROLL_LIFT      2      // Lift the finger
# define P190_ENROLL_PLACE2    3      // Place the finger again (capture 2)
# define P190_ENROLL_STORING   4      // Merging and storing
# define P190_ENROLL_DONE      5      // Stored
                                      // < 0: failed, -<error code>

// Error codes of the <taskname>#EnrollFailed and #Error events, besides the module confirmation codes
# define P190_ERR_NO_RESPONSE  0xFF   // The module didn't answer
# define P190_ERR_FINGER_TIMEOUT 0xFE // No finger placed (or not lifted) within P190_FINGER_TIMEOUT_MS
# define P190_ERR_CANCELLED    0xFD   // fingerprint,cancel
# define P190_ERR_LIBRARY_FULL 0xFC   // fingerprint,enroll without ID: no free slot

// LED color selector of the settings page (here, not in the .ino: PlatformIO adds prototypes of .ino
// functions outside the #ifdef USES_P190, which breaks builds without P190)
void P190_colorSelector(const __FlashStringHelper *label,
                        const __FlashStringHelper *id,
                        int                        selected);

enum class P190_Job : uint8_t {
  None,
  Init,   // Verify password, read the library size and the template count
  Scan,   // Capture, search, report
  Enroll, // Two captures, merge, store
  Delete,
  Empty,
  Count,
  Led,

};

enum class P190_Step : uint8_t {
  None,
  VerifyPassword,
  ReadParams,
  ReadIndex, // Used slots, one page of 256 per command
  ReadCount,
  ScanImage,
  ScanFeatures,
  ScanSearch,
  ScanRemove, // Wait until the finger is lifted (no touch GPIO)
  EnrollImage,
  EnrollFeatures,
  EnrollRemove,
  EnrollModel,
  EnrollStore,
  Delete,
  Empty,
  Led,

};

struct P190_request {
  P190_Job job;
  uint16_t id;
  uint8_t  led[4]; // mode, speed, color, times

};

struct P190_data_struct : public PluginTaskData_base {
public:

  P190_data_struct(struct EventStruct *event);

  P190_data_struct() = delete;
  virtual ~P190_data_struct();

  bool init(struct EventStruct *event);
  void loop(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);

  // Device page: the task values plus the state of the module
  void showValues(struct EventStruct *event) const;

private:

  bool request(const P190_request& req);
  bool enrollPending() const;
  void refresh();
  void setIndex(uint16_t id,
                bool     used);
  void setEnrollState(struct EventStruct *event,
                      int                 state) const;
  void reply(struct EventStruct *event,
             const String      & json) const;
  void replyError(struct EventStruct *event,
                  const String      & message) const;
  void idle(struct EventStruct *event);
  void startJob(struct EventStruct *event,
                const P190_request& req);
  void startJob(struct EventStruct *event,
                P190_Job            job);
  void startStep(P190_Step step,
                 uint32_t  delayMs = 0);
  void sendStep();
  void sendCommand(uint8_t        cmd,
                   const uint8_t *params,
                   size_t         nparams,
                   uint32_t       timeoutMs = P190_ACK_TIMEOUT_MS);
  void handleAck(struct EventStruct       *event,
                 const P190_proto::Packet& pkt);
  void handleTimeout(struct EventStruct *event);
  void handleEnrollAck(struct EventStruct *event,
                       uint8_t             code);
  void report(struct EventStruct *event,
              int                 id,
              int                 score);
  void enrollDone(struct EventStruct *event,
                  int                 errorCode);
  void led(uint8_t   mode,
           uint8_t   color,
           uint8_t   speed,
           uint8_t   times,
           P190_Step next);
  void finish();
  bool touched() const;
  void addEvent(struct EventStruct        *event,
                const __FlashStringHelper *name,
                int                        value) const;
  void fpLog(uint8_t       logLevel,
             const String& message) const;

  ESPeasySerial          *_serial = nullptr;
  P190_proto::Parser      _parser;
  std::deque<P190_request>_queue;

  // Settings
  uint32_t _address      = P190_proto::DEFAULT_ADDRESS;
  uint32_t _password     = 0;
  uint32_t _baud         = 57600;
  int8_t   _touchPin     = -1;
  bool     _ledFeedback  = true;
  uint8_t  _matchColor   = P190_proto::COLOR_BLUE;
  uint8_t  _noMatchColor = P190_proto::COLOR_RED;
  uint16_t _pollMs       = P190_DEF_POLL_MS;
  uint16_t _minScore     = 0;

  // Module state
  bool                _ready    = false;
  uint16_t            _capacity = P190_DEF_CAPACITY;
  int                 _count    = -1;
  std::vector<uint8_t>_index;                  // Used slots, bit n = slot n (P190_proto::indexUsed)
  bool                _indexValid     = false;
  bool                _indexSupported = true;  // The module answers ReadIndexTable
  uint8_t             _indexPage      = 0;
  int                 _enrollSlot     = -1;    // Slot of the last enrolment (fingerprint,list)
  uint32_t            _nextInit       = 0;
  uint32_t            _nextScan       = 0;
  bool                _waitLift       = false; // Don't scan again until the finger is lifted

  // Current job
  P190_Job     _job     = P190_Job::None;
  P190_Step    _step    = P190_Step::None;
  uint32_t     _stepAt  = 0;     // When to send the step
  bool         _waiting = false; // Waiting for the acknowledge of the step
  uint32_t     _sentAt  = 0;
  uint32_t     _timeout = 0;
  P190_request _req{};
  P190_Step    _ledNext        = P190_Step::None;
  uint8_t      _capture        = 1; // Enrolment: capture 1 or 2 (= char buffer)
  uint32_t     _fingerDeadline = 0;
  bool         _cancel         = false;

};

#endif // ifdef USES_P190
