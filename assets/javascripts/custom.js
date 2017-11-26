/* SPDX-License-Identifier: GPL-3.0 */

var codeAtomTypes = {
    TYPE_MASTER: 0,
    TYPE_ABSCHNITT: 1,
    TYPE_PARAGRAPH: 2,
    TYPE_ARTIKEL: 3,
    TYPE_ABSATZ: 4,
    TYPE_SATZ: 5,
    TYPE_NUMMER: 6,
    TYPE_ANHANG: 7,
    TYPE_FUSSNOTE: 8,
    TYPE_TABLE: 9,
    TYPE_TABLE_ROW: 10,
    TYPE_TABLE_CELL: 11,
    TYPE_QUOTE: 12,
    TYPE_TABLE_STOP: 13
};

var controllers = {
    INDEX: 1,
    CODE_INDEX: 2,
    CODE_ATOM: 3,
    CODE_BACK_LINKS: 4,
    NO_SEARCH_RESULTS: 5,
    INTER_CODE_LINKS: 6,
    CODE: 7,
    CODE_ATOM_BATCH_LOOKUP: 8
}

var calcBox = $("<div>").css({
                "position": "absolute",
                "float": "left",
                "white-space": "nowrap",
                "visibility": "hidden",
                "font": "100 13px/13px Helvetica,Arial,Verdana,sans-serif"}).appendTo($("body"));

String.prototype.width = function() {
    calcBox.html(this);
    return calcBox.width();
}

function getInfoText(codeAtomType, key) {
    if(key === "")
        return "";

    switch(codeAtomType) {
        case codeAtomTypes.TYPE_ABSCHNITT:
        return key;

        case codeAtomTypes.TYPE_PARAGRAPH:
        return "\u00A7 " + key;

        case codeAtomTypes.TYPE_ARTIKEL:
        return "Art " + key;

        case codeAtomTypes.TYPE_ABSATZ:
        return "(" + key + ")";

        case codeAtomTypes.TYPE_SATZ:
        return "Satz " + key;

        case codeAtomTypes.TYPE_NUMMER:
        return key + ".";

        case codeAtomTypes.TYPE_ANHANG:
        return "Anlage " + key;

        case codeAtomTypes.TYPE_FUSSNOTE:
        return key;
    }

    return "";
}

var Builder = {
    /* sorts code atoms by position */
    positionComparator: function(codeAtom1, codeAtom2) {
        return parseInt(codeAtom1.position) - parseInt(codeAtom2.position);
    },

    /* sorts code atom links by start value */
    linkStartComparator: function(link1, link2) {
        return parseInt(link1.start) - parseInt(link2.start);
    },

    /* splits text and detects links */
    prepareText: function(node, out, hasSib) {
        var toCodeAtomIds = [], toCodeAtomKeys = [];
        var text = node.text, pos = 0;
        var currentLink = null;

        var textElements = [];

        var key = node.key;
        if(node.atom_type === codeAtomTypes.TYPE_SATZ && !hasSib)
            key = "";
        
        /* first entry contains meta data */
        textElements.push([node.id, node.position, key, node.atom_type, node.depth]);

        $.each(node.links, function(i, link) {
            if(pos < link.start) {
                if(currentLink) {
                    textElements.push([currentLink, 1, toCodeAtomIds, toCodeAtomKeys]);
                    toCodeAtomIds = []; toCodeAtomKeys = []; currentLink = null;
                }

                var words = (text.slice(pos, link.start)).split(" ");
                $.each(words, function(j, word) {textElements.push([word, 0]);});
            }

            if(!currentLink) {
                currentLink = text.slice(link.start, link.stop);
                pos = link.stop;
            }

            toCodeAtomIds.push(link.to_code_atom_id);
            toCodeAtomKeys.push(link.name);
        });

        if(currentLink)
            textElements.push([currentLink, 1, toCodeAtomIds, toCodeAtomKeys]);

        var words = (text.slice(pos)).split(" ");
        $.each(words, function(i, word) {textElements.push([word, 0]);});

        out.push(textElements);
    },

    /* creates index list */
    traverseIndexAtoms: function(codeAtom, fragment) {
        if(codeAtom.atom_type !== codeAtomTypes.TYPE_MASTER) {
            var textNode;
            var found = false;

            switch(codeAtom.atom_type) {
            case codeAtomTypes.TYPE_PARAGRAPH:
                textNode = document.createTextNode("\u00A7 " + codeAtom.key + " " + codeAtom.text); found = true;
                break;
            case codeAtomTypes.TYPE_ARTIKEL:
                textNode = document.createTextNode("Art " + codeAtom.key + " " + codeAtom.text); found = true;
                break;
            case codeAtomTypes.TYPE_ANHANG:
                textNode = document.createTextNode("Anlage " + codeAtom.key + " " + codeAtom.text); found = true;
                break;
            }

            var box;
            if(found) {
                box = document.createElement("div");
                box.className = "indexBox";

                var link = document.createElement("a");
                link.appendChild(textNode);
                link.href = "#";
                $(link).click(function() {
                    var toCodeId = parseInt(this.getAttribute("toCodeId"));
                    var histText = this.getAttribute("histText");

                    window.requestCodeAtom(toCodeId);
                    return false;
                });
                link.setAttribute("toCodeId", codeAtom.id);
                link.setAttribute("histText", getInfoText(codeAtom.atom_type, codeAtom.key));

                box.appendChild(link);
            } else {
                box = document.createElement("div");
                box.className = "indexBoxHeader";
                box.appendChild(document.createTextNode(codeAtom.key + (codeAtom.text !== "" ? " (" + codeAtom.text + ")" : "")));
            }

            fragment.appendChild(box);
        }

        if([codeAtomTypes.TYPE_MASTER, codeAtomTypes.TYPE_ABSCHNITT, codeAtomTypes.TYPE_ABSATZ, codeAtomTypes.TYPE_NUMMER].indexOf(codeAtom.atom_type) !== -1) {
            //sort objects by position
            if(codeAtom.children.length > 1)
                codeAtom.children.sort(this.positionComparator);

            for(var i = 0; i < codeAtom.children.length; i++)
                this.traverseIndexAtoms(codeAtom.children[i], fragment);
        }
    },

    traverseBackLinks: function(records, fragment) {
        for(var i = 0; i < records.length; i++) {
            var record = records[i];

            var box = document.createElement("div");
            box.className = "backLinkBox";

            var link = document.createElement("a");
            link.appendChild(document.createTextNode(record.rev_name));
            link.href = "#";
            $(link).click(function() {
                var toCodeId = parseInt(this.getAttribute("toCodeId"));

                window.requestCodeAtom(toCodeId);
                return false;
            });
            link.setAttribute("toCodeId",record.code_atom_id);

            box.appendChild(link);

            fragment.appendChild(box);
        }
    },

    /* creates intern structure from tree structure */
    traverseAtoms: function(node, out, hasSib) {
        var cLen = node.children.length;

        //sort objects by position
        if(cLen > 1)
            node.children.sort(this.positionComparator);

        if(node.links.length > 1)
            node.links.sort(this.linkStartComparator);

        this.prepareText(node, out, hasSib);

        for(var i = 0; i < cLen; i++)
            this.traverseAtoms(node.children[i], out, cLen>1);

        if(node.atom_type === codeAtomTypes.TYPE_TABLE) {
            var textElements = [];
            /* first entry contains meta data */
            textElements.push([0, 0, "", codeAtomTypes.TYPE_TABLE_STOP, 0]);
            out.push(textElements);
        }
    }
}

var Painter = {
    //contains current code name
    codeName: "",

    //array containing history
    history: [],

    //array containing hidden boxes
    hiddenBoxes: [],

    //keeps track of boxes in the next level
    hiddenBoxId: 0,

    //keeps track of preview boxes
    previewBoxId: 0,

    hideBackLinksBox: function() {
        var backLinksBox = document.getElementById("blBox");

        if(backLinksBox)
           backLinksBox.style.display = "none";
    },

    clearCanvas: function() {
        this.hideBackLinksBox();

        var levelBox = $(".levelBox0").detach();
        this.history.push(levelBox);
    },

    navigateBack: function(steps) {
        this.hideBackLinksBox();

        while(steps > 1) {
            this.history.pop();
            steps--;
        }

        $(".levelBox0").last().remove();
        var levelBox = this.history.pop();

        if(this.history.length === 0) {
            codeName = "";
            $("#title").html(codeName);
        }

        $("#canvas").append(levelBox);
    },

    appendHiddenBoxes: function(fragment) {
        //append hidden boxes
        for(var i = 0; i < this.hiddenBoxes.length; i++)
            fragment.appendChild(this.hiddenBoxes[i]);

        //clear hidden boxes
        this.hiddenBoxes = [];
    },

    initBoxes: function(wrapper, infoBox, textBox, textBoxPadding) {
        wrapper.className = "wrapper";

        infoBox.className = "infoBox";
        infoBox.style.padding = "5px 3px 0px 3px";

        textBox.className = "textBox";
        textBox.style.padding = textBoxPadding;

        wrapper.appendChild(infoBox);
        wrapper.appendChild(textBox);
    },

    draw: function(out, fragment, level, maxLineWidth, drawToBoxId) {
        var link;
        var wrapper;
        var table = null;
        var tableRow = null;
        var tableCell= null;

        for(var i = 0; i < out.length; i++) {
            var textElements = out[i];

            var metaInfo = textElements[0];
            var id           = metaInfo[0];
            var position     = metaInfo[1];
            var key          = metaInfo[2];
            var codeAtomType = metaInfo[3];
            var depth        = metaInfo[4];

            switch(codeAtomType) {
            case codeAtomTypes.TYPE_TABLE:
                table = document.createElement("table");
                table.className = "table";
                continue;
            case codeAtomTypes.TYPE_TABLE_ROW:
                tableRow = document.createElement("tr");
                tableRow.className = "tableRow";
                table.appendChild(tableRow);
                continue;
            case codeAtomTypes.TYPE_TABLE_CELL:
                tableCell = document.createElement("td");
                tableCell.className = "tableCell";
                tableRow.appendChild(tableCell);
                continue;
            case codeAtomTypes.TYPE_TABLE_STOP:
                wrapper = document.createElement("div");
                wrapper.appendChild(table);
                wrapper.className = "wrapper";
                fragment.appendChild(wrapper);
                table = null;
                continue;
            }

            if(table !== null) {
                for(var j = 1; j < textElements.length; j++) {
                    var textElement = textElements[j];
                    var word = textElement[0];

                    if(textElement[1]) {
                        link = document.createElement("a");
                        link.appendChild(document.createTextNode(word + " "));
                        link.href = "#";

                        //get ids
                        var toCodeAtomIds = textElement[2];

                        if(toCodeAtomIds.length > 1) {
                            //get corresponding keys
                            var toCodeAtomKeys = textElement[3];

                            $(link).hover(function() {
                                var previewHoverBox = document.createElement("div");
                                previewHoverBox.className = "previewHoverBox";
                                previewHoverBox.style.top = this.offsetTop + 15;
                                previewHoverBox.style.left = this.offsetLeft;
                                previewHoverBox.appendChild(document.createTextNode("HOVER BOX"));

                                document.getElementById("canvas").appendChild(previewHoverBox);
                            });
                        } else {
                            link.setAttribute("toCodeId",toCodeAtomIds[0]);

                            $(link).click(function() {
                                var toCodeId = parseInt(this.getAttribute("toCodeId"));

                                window.requestCodeAtom(toCodeId);
                                return false;
                            });
                        }
                        tableCell.appendChild(link);
                    } else {
                        tableCell.appendChild(document.createTextNode(word + " "));
                    }
                }
                continue;
            }

            var textBoxPadding = "5px 3px 0px " + (depth*2-1) + "px";

            wrapper = document.createElement("div"); var infoBox = document.createElement("div"); var textBox = document.createElement("div");
            this.initBoxes(wrapper, infoBox, textBox, textBoxPadding);

            infoBox.id = level + "-" + id;

            var infoText = getInfoText(codeAtomType, key);

            if(infoText !== "") {
                link = document.createElement("a");
                link.appendChild(document.createTextNode(infoText));
                link.href = "#";
                $(link).click(function() {
                    var backLinksBox = document.getElementById("blBox");

                    if(!backLinksBox) {
                        backLinksBox = document.createElement("div");
                        backLinksBox.className = "backLinksBox";
                        backLinksBox.id = "blBox";

                        document.getElementById("canvas").appendChild(backLinksBox);
                    }

                    backLinksBox.innerHTML = "";
                    backLinksBox.style.display = "none";
                    backLinksBox.style.top = this.offsetTop + 15;
                    backLinksBox.style.left = this.offsetLeft;

                    var backLinksBoxHeader = document.createElement("div");
                    backLinksBoxHeader.className = "backLinksBoxHeader";
                    backLinksBoxHeader.appendChild(document.createTextNode("Verweise auf " + this.text + ":"));

                    backLinksBox.appendChild(backLinksBoxHeader);

                    var toCodeId = parseInt(this.getAttribute("toCodeId"));

                    window.requestBackLinks(toCodeId);

                    return false;
                });
                link.setAttribute("toCodeId",id);

                infoBox.appendChild(link);
            }

            //check if code atom has text
            if([codeAtomTypes.TYPE_ABSCHNITT, codeAtomTypes.TYPE_PARAGRAPH, codeAtomTypes.TYPE_ARTIKEL, codeAtomTypes.TYPE_SATZ].indexOf(codeAtomType) === -1) {
                fragment.appendChild(wrapper);
                continue;
            }

            var currentLineWidth = depth+2+1;

            for(var j = 1; j < textElements.length; j++) {
                var textElement = textElements[j];

                //get the first word
                var word = textElement[0];

                //calculate the width, +1 for space
                var currentWordWidth = word.width() + 1;

                //check if current line width exceeds max line width
                if(currentLineWidth + currentWordWidth >= maxLineWidth) {
                    fragment.appendChild(wrapper);
                    if(this.hiddenBoxes.length)
                        this.appendHiddenBoxes(fragment);

                    wrapper = document.createElement("div"); infoBox = document.createElement("div"); textBox = document.createElement("div");
                    this.initBoxes(wrapper, infoBox, textBox, textBoxPadding);

                    currentLineWidth = depth+2-1 + currentWordWidth;
                } else {
                    currentLineWidth += currentWordWidth;
                }

                //check if word is a link
                if(textElement[1]) {
                    var boxId;
                    //next level
                    var nextLevel = level + 1;
                    //create link
                    link = document.createElement("a");
                    link.appendChild(document.createTextNode(word));
                    link.href = "#";
                    $(link).click(function() {
                        linkCallback(this);
                        return true;
                    });

                    //get ids
                    var toCodeAtomIds = textElement[2];

                    if(toCodeAtomIds.length > 1) {
                        //get corresponding keys
                        var toCodeAtomKeys = textElement[3];

                        boxId = "pvb" + level + "-" + this.previewBoxId;

                        var previewBox = document.createElement("div");
                        previewBox.id = boxId;
                        previewBox.className = "previewBox" + level;
                        previewBox.setAttribute("level",level);

                        for(var k = 0; k < toCodeAtomIds.length; k++) {
                            var pvbWrapper = document.createElement("div"); var pvbInfoBox = document.createElement("div"); var pvbTextBox = document.createElement("div");
                            this.initBoxes(pvbWrapper, pvbInfoBox, pvbTextBox, textBoxPadding);
                            pvbInfoBox.appendChild(document.createTextNode("\u25CF"));
                            pvbInfoBox.style.textAlign = "right";

                            var pvbBoxId = "lvl" + nextLevel + "-" + this.hiddenBoxId;

                            var pvbLink = document.createElement("a");
                            pvbLink.appendChild(document.createTextNode(toCodeAtomKeys[k]));
                            pvbLink.href = "#";
                            $(pvbLink).click(function() {
                                linkCallback(this);
                                return true;
                            });
                            pvbLink.setAttribute("toCodeId",toCodeAtomIds[k]);
                            pvbLink.setAttribute("boxId",pvbBoxId);

                            var span = document.createElement("span");
                            span.className = "link";
                            span.appendChild(pvbLink)

                            pvbTextBox.appendChild(span);
                            previewBox.appendChild(pvbWrapper);

                            //create hidden box
                            var pvbHiddenBox = document.createElement("div");
                            pvbHiddenBox.id = pvbBoxId;
                            pvbHiddenBox.className = "levelBox" + nextLevel;
                            pvbHiddenBox.setAttribute("level",nextLevel);
                            previewBox.appendChild(pvbHiddenBox);

                            this.hiddenBoxId++;
                        }

                        this.hiddenBoxes.push(previewBox);
                        this.previewBoxId++;
                    } else {
                        boxId = "lvl" + nextLevel + "-" + this.hiddenBoxId;

                        var hiddenBox = document.createElement("div");
                        hiddenBox.id = boxId;
                        hiddenBox.className = "levelBox" + nextLevel;
                        hiddenBox.setAttribute("level",nextLevel);

                        this.hiddenBoxes.push(hiddenBox);
                        this.hiddenBoxId++;

                        link.setAttribute("toCodeId",toCodeAtomIds[0]);
                    }

                    link.setAttribute("boxId",boxId);

                    //append link
                    var span = document.createElement("span");
                    span.className = "link";
                    span.appendChild(link);

                    textBox.appendChild(span);
                } else {
                    //append word
                    textBox.appendChild(document.createTextNode(word + " "));
                }
            }

            fragment.appendChild(wrapper);

            if(this.hiddenBoxes.length)
                this.appendHiddenBoxes(fragment);
        }

        /* check type of top code atom */
        if(out[0][0][3] === codeAtomTypes.TYPE_PARAGRAPH)
            return;

        /* append (Mehr) link */
        wrapper = document.createElement("div");
        wrapper.className = "wrapper";

        var showMoreBox = document.createElement("div");
        showMoreBox.className = "showMoreBox";

        link = document.createElement("a");
        link.appendChild(document.createTextNode("(Mehr)"));
        link.href = "#";
        $(link).click(function() {
            var boxId = this.getAttribute("boxId");

            var box = document.getElementById(boxId);
            box.innerHTML = "";

            var toCodeId = parseInt(this.getAttribute("toCodeId"));

            window.window.requestCodeAtomInline(toCodeId, function(response) {
                Painter.drawTextToBox(response, boxId, /*TODO*/1);
            });
            return true;
        });
        link.setAttribute("toCodeId",out[0][0][0]);
        link.setAttribute("boxId",drawToBoxId);

        showMoreBox.appendChild(link);
        wrapper.appendChild(showMoreBox);

        fragment.appendChild(wrapper);
    },

    drawCodesHeader: function(hightlightChar) {
        var levelBox = document.createElement("div");
        levelBox.className = "levelBox0";
        levelBox.setAttribute("level",0);

        var indexHeader = document.createElement("div");
        indexHeader.className = "codeHeader";
        var indexBody = document.createElement("div");
        indexBody.id = "indexBody";

        var index = "ABCDEFGHIJKLMNOPQRSTUVWYZ123456789".split("");
        $.each(index, function(i, c) {
            var link = document.createElement("a");
            link.appendChild(document.createTextNode(c));
            link.href = "#";
            link.className = "indexLink " +
                    (hightlightChar.toLowerCase() === c.toLowerCase() ? "hl" : "");
            $(link).click(function() {
                window.requestIndex(this.getAttribute("indexChar"));
                return false;
            });
            link.setAttribute("indexChar",c);

            var span = document.createElement("span");
            span.className = "link";
            span.appendChild(link);

            indexHeader.appendChild(span);
        });

        levelBox.appendChild(indexHeader);
        levelBox.appendChild(indexBody);

        document.getElementById("canvas").appendChild(levelBox);
    },

    drawCodes: function(records) {
        var indexBody = document.getElementById("indexBody");
        indexBody.innerHTML = "";

        for(var i = 0; i < records.length; i++) {
            var record = records[i];

            var wrapper = document.createElement("div");
            wrapper.className = "wrapper";

            var shortBox = document.createElement("div");
            shortBox.className = "shortBox";

            shortBox.appendChild(document.createTextNode(record.short));

            var codeBox = document.createElement("div");
            codeBox.className = "codeBox";

            var link = document.createElement("a");
            var linkLabel = record.name;

            link.appendChild(document.createTextNode(linkLabel));
            link.href = "#";
            $(link).click(function() {
                var codeId = parseInt(this.getAttribute("codeId"));
                window.requestCodeIndex(codeId);
                return false;
            });
            link.setAttribute("codeId",record.id);
            link.setAttribute("name", record.name);
            link.setAttribute("short", record.short);

            codeBox.appendChild(link);

            wrapper.appendChild(shortBox);
            wrapper.appendChild(codeBox);

            indexBody.appendChild(wrapper);
        }
    },

    drawIndex: function(records) {
        var levelBox = document.createElement("div");
        levelBox.className = "levelBox0";
        levelBox.setAttribute("level",0);

        var fragment = document.createDocumentFragment();

        Builder.traverseIndexAtoms(records, fragment);

        levelBox.appendChild(fragment);

        document.getElementById("canvas").appendChild(levelBox);
    },

    drawBackLinks: function(records) {
        if(records.length > 0) {
            var fragment = document.createDocumentFragment();
            Builder.traverseBackLinks(records, fragment);

            var backLinksBox = document.getElementById("blBox");
            backLinksBox.appendChild(fragment);
            backLinksBox.style.display = "block";
        }
    },

    drawText: function(records) {
        var levelBox = document.createElement("div");
        levelBox.className = "levelBox0";
        levelBox.setAttribute("level",0);
        levelBox.id = "lvl0-0";
        document.getElementById("canvas").appendChild(levelBox);

        var fragment = document.createDocumentFragment();

        var out = [];
        Builder.traverseAtoms(records, out, true);
        this.draw(out, fragment, 0, $(levelBox).width() - 150, "lvl0-0");

        levelBox.appendChild(fragment);
    },

    drawTextToBox: function(records, boxId, scrollToId) {
        var box = document.getElementById(boxId);
        var level = parseInt(box.getAttribute("level"));

        var fragment = document.createDocumentFragment();

        var out = [];
        Builder.traverseAtoms(records, out, true);
        this.draw(out, fragment, level, $(box).width() - 150, boxId);

        box.appendChild(fragment);

        if(!scrollToId)
            return;

        var scrollTo = document.getElementById(level + "-" + scrollToId);
        scrollTo.scrollIntoView();
    },

    showFailedSearch: function(query) {
        var results = $("<div class=\"levelBox0\">");

        var header = $("<div class=\"search-header\">");
        header.append("<span>Ihre Suche nach \"</span>");
        header.append($("<span>").text(query).html());
        header.append("<span>\" ergab keine Treffer.</span>");

        var suggestion = $("<div>Stattdessen:</div>");
        var list = $("<li>");

        var codeIndexFn = function() {
            window.requestIndex($(this).data("codeIndexStartChar"));
        }

        var tokens = query.split(" ");

        if(tokens.length > 0) {
            $.each(tokens, function(idx, token) {
                var firstChar = token[0];
                var toIndexItem = $("<ul class=\"search-sugg\">" + firstChar.toUpperCase() +"-Index</ul>");
                toIndexItem.data("codeIndexStartChar", firstChar.toLowerCase());
                toIndexItem.click(codeIndexFn);
                list.append(toIndexItem);
            });
        } else {
            var toIndexItem = $("<ul class=\"search-sugg\">Zurück zu den Büchern.</ul>");
            toIndexItem.data("codeIndexStartChar", "a");
            toIndexItem.click(codeIndexFn);
            list.append(toIndexItem);
        }

        results.append(header);
        results.append(suggestion);
        results.append(list);
        $("#canvas").append(results);
    },

    drawInterCodeLinks: function(links, fromID, readyCB) {
        var main = $("<div class=\"levelBox0\">");

        var header = $("<div class=\"icl-header\">" +
                         "<div class=\"icl-title\">Bezüge zwischen:</div>" +
                         "<div class=\"icl-vs-box\">" +
                           "<div id=\"icl-code-from\" class=\"like-link\"></div>" +
                           "<div>und</div>" +
                           "<div id=\"icl-code-to\" class=\"like-link\"></div>" +
                         "</div>" +
                       "</div>");
        main.append(header);

        var tableHeader = $("<div class=\"table icl-table\">" +
                              "<div class=\"tbl-head\">" +
                                "<div class=\"tbl-cell icl-col-1\" id=\"icl-code-from-short\"></div>" +
                                "<div class=\"tbl-cell icl-col-2\"></div>" +
                                "<div class=\"tbl-cell icl-col-3\" id=\"icl-code-to-short\"></div>" +
                              "</div>" +
                            "</div>");

        var ids = [];
        $.each(links["records"], function(i, link) {
            ids.push(link["code_atom_id"]);
            ids.push(link["to_code_atom_id"]);
        });

        ids = ids.filter(function(value, i, self) {
            return self.indexOf(value) === i;
        });

        var createQualifiedName = function(id, response) {
            var records = response[id];

            if(!records) {
                return "";
            }

            /* Must slice, otherwise we always switch the original records. */
            records = records.slice().reverse();

            var tokens = [];
            var hasPhrase = false;

            $.each(records, function(i, record) {
                switch(record.atom_type) {
                    case codeAtomTypes.TYPE_PARAGRAPH:
                        tokens.push("\u00A7 " + record.key + " <span class=\"name\">" + record.text + "</span>");
                        break;
                    case codeAtomTypes.TYPE_ARTIKEL:
                        tokens.push("Art. " + record.key);
                        break;
                    case codeAtomTypes.TYPE_ABSATZ:
                        tokens.push("Abs. " + record.key);
                        break;
                    case codeAtomTypes.TYPE_SATZ:
                        /* Skip some nested things ... too deep to be useful. */
                        if(!hasPhrase) {
                            hasPhrase = true;
                            tokens.push("Satz " + record.key);
                        }
                        break;
                    case codeAtomTypes.TYPE_NUMMER:
                        tokens.push("Nr. " + record.key);
                        break;
                    case codeAtomTypes.TYPE_ANHANG:
                        tokens.push("Anlage " + record.key + " <span class=\"name\">" + record.text + "</span>");
                        break;
                }
            });

            return tokens.join(" ");
        };

        window.requestBatchCodeAtomLookup(ids, function(response) {
            var groupedLinks = window.groupInterCodeLinks(links["records"], fromID);

            var lastFrom = null;
            $.each(groupedLinks, function(i, link) {
                var from = "", to = "", direction = "";
                var isFromLink = fromID === link["code_id"];

                if(isFromLink) {
                    if(lastFrom === link["code_atom_id"]) {
                        to = link["to_code_atom_id"];
                    } else if(lastFrom === link["to_code_atom_id"]) {
                        from = link["code_atom_id"];
                    } else {
                        direction = "&#8594;";
                        to = link["to_code_atom_id"];
                        from = link["code_atom_id"];
                    }
                } else {
                    if(lastFrom === link["code_atom_id"]) {
                        from = link["to_code_atom_id"];
                    } else if(lastFrom === link["to_code_atom_id"]) {
                        to = link["code_atom_id"];
                    } else {
                        direction = "&#8592;";
                        to = link["code_atom_id"];
                        from = link["to_code_atom_id"];
                    }
                }

                tableHeader.append("<div class=\"tbl-row\">" +
                                     "<div class=\"tbl-cell icl-col-1\">" + createQualifiedName(from, response) + "</div>" +
                                     "<div class=\"tbl-cell icl-col-2\">" + direction + "</div>" +
                                     "<div class=\"tbl-cell icl-col-3\">" + createQualifiedName(to, response) + "</div>" +
                                   "</div>");

                lastFrom = link["code_atom_id"];
            });
        });

        main.append(tableHeader);

        $("#canvas").append(main);
        readyCB();
    },

    drawFromCodeForInterCodeLinks: function(code) {
        $("#icl-code-from").text(code.name + " (" + code.short_name + ") ");
        $("#icl-code-from").click(function() {
            requestCodeIndex(code.id);
        });
        $("#icl-code-from-short").text(code.short_name);
    },

    drawToCodeForInterCodeLinks: function(code) {
        $("#icl-code-to").text(code.name + " (" + code.short_name + ") ");
        $("#icl-code-to").click(function() {
            requestCodeIndex(code.id);
        });
        $("#icl-code-to-short").text(code.short_name);
    }
}

window.LINK_PARAGRAPH_EXTRACT_RE = /^(?:§|Art)\s*(\d+\w*)/;
window.LINK_KEY_SPLIT_RE = /^(\d+)(\w*)/;

/* Returns something like (by array order!):

   § 1 AG -> § X BG
   § 1a AG -> § X BG
   § 1a AG -> § Y BG
   § 2 BG -> § Z AG
    ...
*/
window.groupInterCodeLinks = function(links, fromID) {
    var keyedLinks = [];

    /* Extract paragraph/article from rev_name. */
    $.each(links, function(i, link) {
        var key = link.rev_name.match(window.LINK_PARAGRAPH_EXTRACT_RE);

        if(key) {
            keyedLinks.push($.extend(link, { "sortKey": key[1] }));
        }
    });

    /* Sort by numeric part, then by suffix, then give fromID priority. */
    var keySortFn = function(a, b) {
        var componentsA = a["sortKey"].match(window.LINK_KEY_SPLIT_RE);
        var componentsB = b["sortKey"].match(window.LINK_KEY_SPLIT_RE);

        if(componentsA && componentsB) {
            var intA = parseInt(componentsA[1]);
            var intB = parseInt(componentsB[1]);

            if(intA !== intB) {
                return (intA > intB ? 1 : -1);
            } else {
                var suffixA = componentsA[2];
                var suffixB = componentsB[2];

                if(suffixA === suffixB) {
                    if(a["code_id"] === b["code_id"]) {
                        return 0;
                    } else {
                        return (a["code_id"] === fromID ? 1 : -1);
                    }
                } else {
                    return suffixA > suffixB ? 1 : -1;
                }
            }
        }

        return 0;
    };

    return keyedLinks.sort(keySortFn);
}

function linkCallback(e) {
    /* get selected box id */
    var boxId = e.getAttribute("boxId");

    /* get corresponding box */
    var box = document.getElementById(boxId);

    /* get selected box level */
    var boxLevel = parseInt(box.getAttribute("level"));

    /* get code atom id */
    var toCodeId = parseInt(e.getAttribute("toCodeId"));

    /* check level */
    if(boxLevel === 3) {
        var histText = "";

        for(var i = 1; i < 3; i++) {
            box = $(e).closest(".levelBox" + i);
            histText += box.attr("linkName") + " / ";
        }

        histText += e.innerHTML;

        window.requestCodeAtom(toCodeId);
        return;
    }

    /* cache link name */
    box.setAttribute("linkName", e.innerHTML);

    /* retrieve text box */
    var textBox = $(e).closest(".textBox");

    /* get all links inside the text box */
    var links = textBox.find("a");

    /* get visibility */
    var boxDisplay = box.style.display;

    /* hide all corresponding boxes */
    for(var i = 0; i < links.length; i++) {
        var span = links[i].parentNode;
        $(span).removeClass("link-level" + boxLevel);

        var currentLink = links[i];
        var currentBoxId = currentLink.getAttribute("boxId");
        var currentBox = document.getElementById(currentBoxId);
        currentBox.style.display = "";
    }

    if(boxDisplay === "") {
        /* get span */
        var span = e.parentNode;
        $(span).addClass("link-level" + boxLevel);

        box.style.display = "block";
    }

    /* check if box has content */
    if(!box.hasChildNodes()) {
        var loader = $("<div class=\"inline-loader\">Lädt ...</div>");
        $(box).append(loader);

        window.requestCodeAtomInline(toCodeId, function(response) {
            loader.remove();
            Painter.drawTextToBox(response, boxId, /*TODO*/ 1);
        });
    }
}

/*
 *
 * The following functions are called from Qt
 *
 */

function navigateBack(steps, scrollBarValue) {
    Painter.navigateBack(steps);

    window.scrollTo(0, scrollBarValue);
}

function drawBackLinks(json) {
    var obj = $.parseJSON(json);
    Painter.drawBackLinks(obj["records"]);
}

function drawText(json, boxId, scrollToId) {
    var obj = $.parseJSON(json);

    if(!boxId) {
        Painter.clearCanvas();
        Painter.drawText(obj);
    } else {
        Painter.drawTextToBox(obj, boxId, scrollToId);
    }
}

$("body").click(function() {
    Painter.hideBackLinksBox();
    return false;
});

window.setLoadMaskDisplayed = function(state) {
    var css = state ? "": "none";
    $("#load-mask").css("display", css);
    $("#load-mask-text").css("display", css);
}

window.requestIndex = function(startsWith) {
    setLoadMaskDisplayed(true);
    doRequest({
        "controllerID": controllers.INDEX,
        "args": {
            "startsWith": startsWith
        },
        "success": function(response, requestID) {
            Painter.clearCanvas();
            Painter.drawCodesHeader(startsWith);
            Painter.drawCodes(response["records"]);
            window.setLoadMaskDisplayed(false);
            window.scrollTo(0, 0);

            cache[requestID] = $(".levelBox0").first();
        },
        "cachedLoad": window.defaultCachedLoad
    });
}

window.requestCodeIndex = function(codeID) {
    setLoadMaskDisplayed(true);
    doStackingRequest({
        "controllerID": controllers.CODE_INDEX,
        "prevControllerID": controllers.INDEX,
        "args": {
            "codeID": codeID
        },
        "success": function(response, requestID) {
            Painter.clearCanvas();
            Painter.drawIndex(response);

            var levelBox = $(".levelBox0").first();
            cache[requestID] = levelBox;

            window.setLoadMaskDisplayed(false);
            window.scrollTo(0,0);
        },
        "cachedLoad": window.defaultCachedLoad
    });
}

window.requestCodeAtom = function(codeAtomID) {
    setLoadMaskDisplayed(true);
    doStackingRequest({
        "controllerID": controllers.CODE_ATOM,
        "prevControllerID": controllers.CODE_INDEX,
        "args": {
            "codeAtomID": codeAtomID
        },
        "success": function(response, requestID) {
            Painter.clearCanvas();
            Painter.drawText(response);

            var levelBox = $(".levelBox0").first();
            cache[requestID] = levelBox;

            window.setLoadMaskDisplayed(false);
            window.scrollTo(0,0);
        },
        "cachedLoad": window.defaultCachedLoad
    });
}

window.requestCodeAtomInline = function(codeAtomID, cb) {
    doRequest({
        "controllerID": controllers.CODE_ATOM,
        "args": {
            "codeAtomID": codeAtomID
        },
        "takeover": false,
        "success": cb
    });
}

window.requestBackLinks = function(codeAtomID) {
    doRequest({
        "controllerID": controllers.CODE_BACK_LINKS,
        "args": {
            "codeAtomID": codeAtomID
        },
        "takeover": false,
        "success": function(response, requestID) {
            Painter.drawBackLinks(response["records"]);
        }
    });
}

window.requestBatchCodeAtomLookup = function(codeAtomIDs, cb) {
    doRequest({
        "controllerID": controllers.CODE_ATOM_BATCH_LOOKUP,
        "args": {
            "codeAtomIDs": codeAtomIDs.join(",")
        },
        "takeover": false,
        "success": cb
    });
}

window.showInterCodeLinks = function(fromID, toID) {
    var syncObject = {
        "linksLoaded": false,
        "fromLoaded": false,
        "toLoaded": false,
        "everythingLoaded": function() {
            return this.linksLoaded && this.fromLoaded && this.toLoaded;
        }
    };

    setLoadMaskDisplayed(true);
    doRequest({
        "controllerID": controllers.INTER_CODE_LINKS,
        "args": {
            "fromCodeID": fromID,
            "toCodeID": toID,
        },
        "takeover": true,
        "success": function(response, requestID) {
            Painter.clearCanvas();
            Painter.drawInterCodeLinks(response, fromID, function() {
                syncObject.linksLoaded = true;
                setLoadMaskDisplayed(!syncObject.everythingLoaded());
            });

            if(syncObject.fromLoaded) {
                Painter.drawFromCodeForInterCodeLinks(syncObject.fromCode);
            }
            if(syncObject.toLoaded) {
                Painter.drawToCodeForInterCodeLinks(syncObject.toCode);
            }
            cache[requestID] = $(".levelBox0").first();
        },
        "cachedLoad": window.defaultCachedLoad
    });

    doRequest({
        "controllerID": controllers.CODE,
        "args": {
            "codeID": fromID
        },
        "takeover": false,
        "success": function(response, requestID) {
            if(syncObject.linksLoaded) {
                Painter.drawFromCodeForInterCodeLinks(response);
            } else {
                syncObject.fromCode = response;
            }
            syncObject.fromLoaded = true;
            setLoadMaskDisplayed(!syncObject.everythingLoaded());
        }
    });

    doRequest({
        "controllerID": controllers.CODE,
        "args": {
            "codeID": toID
        },
        "takeover": false,
        "success": function(response, requestID) {
            if(syncObject.linksLoaded) {
                Painter.drawToCodeForInterCodeLinks(response);
            } else {
                syncObject.toCode = response;
            }
            syncObject.toLoaded = true;
            setLoadMaskDisplayed(!syncObject.everythingLoaded());
        }
    });
}

window.cache = {};
window.pendingRequests = {};

window.prepareRequest = function(opts, replayable) {
    var guid = function() {
        var random = function () {
            return (((1+Math.random())*0x10000)|0).toString(16).substring(1);
        }
        return random() + random() + random() + random();
    }

    var requestID = guid();

    pendingRequests[requestID] = {
        "replayable": replayable,
        "success": opts.success || function() {},
        "failure": opts.failure || function() {},
        "handler": opts.handler || window,
        "cacheCheck": opts.cachedLoad || function() { return false; }
    };

    return requestID;
}

window.doRequest = function(opts) {
    var requestID = prepareRequest(opts, opts.takeover !== false);
    WebDispatcher.do_request(requestID, opts.controllerID, opts.args || {}, opts.takeover !== false);
}

window.doStackingRequest = function(opts) {
    var requestID = prepareRequest(opts, true);
    WebDispatcher.do_stacking_request(requestID, opts.controllerID,
                                      opts.prevControllerID,
                                      opts.args || {},
                                      opts.prev_args || {});
}

window.defaultCachedLoad = function(requestID) {
    var inCache = !!cache[requestID];

    if(inCache) {
        Painter.clearCanvas();
        var levelBox = cache[requestID];
        $("#canvas").append(levelBox);
    }

    return inCache;
}

window.checkIfRequestNeeded = function(requestID) {
    var callback = pendingRequests[requestID];
    var needed = false;

    if(callback && callback.cacheCheck) {
        needed = !(callback.cacheCheck.apply(callback.handler, [requestID]));
    }

    if(needed) {
        window.setLoadMaskDisplayed(true);
    }

    return needed;
}

window.requestResponseCallback = function(requestID, code, response) {
    var callback = pendingRequests[requestID];

    if(callback) {
        var json = $.parseJSON(response);
        callback[code === 0 ? "success" : "failure"].apply(callback.handler, [json, requestID]);

        /* Inline request: instant deletion. */
        if(!callback["replayable"]) {
            window.disposeReplayableRequest(requestID);
        }
    }
}

window.disposeReplayableRequest = function(requestID) {
    delete cache[requestID];
    delete pendingRequests[requestID];
}

window.appShowRequest = function(ctrlID, args) {
    switch(ctrlID) {
        case controllers.INDEX:
            window.requestIndex(args["startsWith"] || "a");
            break;
        case controllers.CODE_INDEX:
            window.requestCodeIndex(args["codeID"] || 1);
            break;
        case controllers.CODE_ATOM:
            window.requestCodeAtom(args["codeAtomID"]);
            break;
        case controllers.NO_SEARCH_RESULTS:
            Painter.clearCanvas();
            Painter.showFailedSearch(args["query"]);
            break;
        case controllers.INTER_CODE_LINKS:
            window.showInterCodeLinks(args["fromCodeID"], args["toCodeID"]);
            break;
        default:
            console.log("Invalid ctrl request from app.");
            break;
    }
}

window.letTheShowBegin = function() {
    requestIndex("a");
}
