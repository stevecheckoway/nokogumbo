local util = require "gumbo.dom.util"

local Comment = util.merge("CharacterData", {
    type = "comment",
    nodeName = "#comment",
    nodeType = 8
})

function Comment:cloneNode()
    return setmetatable({data = self.data}, Comment)
end

return Comment
