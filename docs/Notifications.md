# Notification settings

This document describes the `notifications` key in the config file. 

## Always-applicable keys

* `provider`: one of `"native"`, `"ntfy"`, or `null` to disable all notifications.
  Example:
  ```json
  "notifications": {
    "provider": "native"
  },
  ```

## Provider-specific config options

### `ntfy`

Used for [ntfy.sh](https://ntfy.sh/)-based notifications.

* `url`: the URL to the ntfy instance, including the topic.
* `bearer`: a bearer token. This is the only form of supported authentication. It's assumed you _need_ auth to use ntfy to simplify the implementation

Example:
```json
"notifications": {
  "provider": "ntfy",
  "url": "https://ntfy.example.com/data-dumps",
  "bearer": "seekrit_token_1234"
},
```
