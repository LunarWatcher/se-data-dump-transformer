from time import monotonic
from selenium.webdriver import ActionChains
from selenium.webdriver.common.by import By
from sedd.watcher.recovery import LastState
from loguru import logger
from selenium.webdriver.firefox.webdriver import WebDriver

def native_retry(
    browser: WebDriver,
    last_sizes: dict[str, LastState],
    path: str,
):
    """
    This function uses about:downloads to attempt a soft/resuming retry.

    As part of the 2026-03-31 download clusterfuck where SE broke the
    stackoverflow.com download, the link was expanded from being valid for 30
    seconds to 24 hours. This means we now can use the browser retry to restart
    on failure. Unfortunately, selenium does not offer an API for this, so we
    need to do it ourselves.
    """
    logger.info(
        "{} is within the 24 hour link validity "
        "window. Attempting restart via browser "
        "facilities. This is soft restart {} out of max 5",
        path,
        last_sizes[path].soft_restart_count + 1
    )
    browser.get("about:downloads")
    elems = browser.find_elements(
        By.CSS_SELECTOR,
        # Last seen in Firefox 153.0.4
        # There's also the downloadIconRetry class, but not sure
        # how stable it is
        "button[data-l10n-id=\"downloads-cmd-retry\"]",
    )
    if len(elems) == 0:
        logger.error(
            "Download has failed, but no restart buttons "
            "available. Triggering soft retry fail "
            "condition. The download will now restart."
        )
        last_sizes[path].soft_restart_count += 5
    else:
        logger.info("Found {} failed downloads", len(elems))
        # Soft restart count is used just in case the
        # retries instantly fail. We have to cap the soft
        # restart list to avoid infinite failed soft
        # retries, and parsing the rest of the page DOM is
        # too annoying to bother.
        # TODO: can we replace this with some API that I
        # couldn't find? We use vAPI for uBlock origin for
        # example - does about:downloads or any other part
        # of Firefox give us that ability?
        last_sizes[path].soft_restart_count += 1
        # Invalidate the last observed size so soft retries
        # that fail and become hard retries don't cause a
        # mess
        last_sizes[path].last_observed_size = 0
        last_sizes[path].last_observed_change = monotonic()
        for elem in elems:
            ActionChains(browser) \
                .move_to_element(elem) \
                .click() \
                .perform()
