# coding=utf-8



from urllib.parse import urljoin
import re
import requests
from requests_html import HTMLSession


class Store:
    __slots__=["pic", "name", "addr", "phone", "hours", "url"]

    def __str__(self):
        # return str(self.__slots__)
        return ", ".join(["{}: {}".strip().format(k, getattr(self, k, "")) for k in self.__slots__])

    def __repr__(self):
        return self.__str__()

def get_info_from_page(url, info):
    session = HTMLSession()
    r = session.get(url)
    boxes = r.html.find("div.view-id-store_locations_by_state")[0].find("div.views-row")
    for box in boxes:
        if not box.full_text.strip():
            continue
        # print(box.full_text)
        s = Store()
        s.pic = urljoin(url, box.find("div.views-field-field-storefront-image")[0].find("img")[0].attrs["src"])
        s.url = urljoin(url, box.find("div.views-field-field-storefront-image")[0].find("a")[0].attrs["href"])
        s.name = box.find("div.views-field-title")[0].full_text.strip()
        s.addr = ""
        addr_div = box.find("div.views-field-field-postal-address")[0].find("div.field-content", first=True)
        s.addr = addr_div.find("div.street-block", first=True).full_text.replace(",", " ")
        s.addr += addr_div.find("div.locality-block", first=True).full_text.replace(",", " ")
        s.phone = box.find("div.views-field-field-phone-number")[0].find("div.field-content")[0].full_text.strip()
        s.phone = s.phone.replace("(", "").replace(")", "").replace(" ", "-").replace(".", "-")
        s.hours = box.find("div.views-field-field-store-hours")[0].find("div.field-content")[0].full_text.strip().replace(",", " ")
        info.append(s)

URL="https://www.wholefoodsmarket.com/stores/list/state?page="



if __name__ == "__main__":
    l = []
    for i in range(1, 23):
        get_info_from_page(URL+str(i), l)
        print("page {}, total {} stores".format(i, len(l)))
        # time.sleep(2)


    with open("out.csv", "wb") as ofile:
        ofile.write(b", ".join([b"pic", b"name", b"addr", b"phone", b"hours", b"url"]))
        ofile.write(b"\n")

        for s in l:
            ofile.write(b", ".join([s.pic.encode("utf-8"), s.name.encode("utf-8"), s.addr.encode("utf-8"), s.phone.encode("utf-8"), s.hours.encode("utf-8"), s.url.encode("utf-8")]))
            ofile.write(b"\n")

