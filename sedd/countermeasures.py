from traceback import print_exception
from lxml import html
from lxml.etree import ParserError
from lxml.html import builder
from gzip import compress, decompress

bot_cloak_script = """
<script>
console.log("Deploying countermeasures");
Object.defineProperty(navigator, 'webdriver', {
  get: () => undefined
});
</script>
"""

def bot_cloak(req, res):
    try:
        if (res.headers is None
            or (res.headers["Content-Type"] and "html" not in
                res.headers["Content-Type"].lower())
            or res.headers['Content-Encoding'] != 'gzip'):
            return None
        print("Passed")
        try:
            parsed_html = html.fromstring(decompress(res.body))
        except ParserError:
            print("Failed")
            return None
        try:
            parsed_html.head.insert(0, html.fragment_fromstring(
                bot_cloak_script
            ))
        except IndexError:
            print("Failed")
            return None
        print("Injected")
        # This errors hard with pyright, but it's fine. Pyright checks all the
        # return types as possible, even though this configuration returns bytes.
        res.body = compress(html.tostring(parsed_html, method = "html",
                                      encoding = "utf-8")) # type: ignore
    except Exception as e:
        # Selenium-wire-2 eats exceptions for breakfast, so this is needed to
        # get anything
        print_exception(e)
